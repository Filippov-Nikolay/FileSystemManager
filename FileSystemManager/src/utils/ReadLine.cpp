#include "ReadLine.h"

#include <iostream>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace {

size_t FindWordStart(const std::string& buffer)
{
    size_t wordStart = 0;
    bool inQuotes = false;

    for (size_t i = 0; i < buffer.size(); ++i)
    {
        if (buffer[i] == '"')
            inQuotes = !inQuotes;
        else if (buffer[i] == ' ' && !inQuotes)
            wordStart = i + 1;
    }

    return wordStart;
}

void EraseChars(size_t count)
{
    for (size_t i = 0; i < count; ++i)
        std::cout << "\b \b";
}

void ReplaceBuffer(std::string& buffer, const std::string& newContent)
{
    EraseChars(buffer.size());
    buffer = newContent;
    std::cout << buffer << std::flush;
}

} // namespace

std::string ReadLine(const std::string& prompt,
                     const Completer& completer,
                     std::vector<std::string>* history)
{
    std::cout << prompt << std::flush;

    std::string buffer;
    std::string savedInput; // preserves current input while navigating history

    bool wasTab = false;
    std::string completionBase;
    size_t completionIndex = 0;

    // historyIndex == history->size() means "current (unsaved) input"
    size_t historyIndex = history ? history->size() : 0;

    // Shared lambda: handles tab completion logic (identical on both platforms)
    auto handleTab = [&]()
    {
        if (!completer) return;

        const size_t wordStart = FindWordStart(buffer);
        const bool hasOpenQuote = (wordStart < buffer.size() && buffer[wordStart] == '"');

        if (!wasTab)
        {
            completionBase = hasOpenQuote
                ? buffer.substr(wordStart + 1)
                : buffer.substr(wordStart);
            completionIndex = 0;
            wasTab = true;
        }

        const auto completions = completer(completionBase, buffer.substr(0, wordStart));
        if (completions.empty()) return;

        EraseChars(buffer.size() - wordStart);

        const std::string& completion = completions[completionIndex % completions.size()];
        completionIndex++;

        const bool needsQuotes = completion.find(' ') != std::string::npos;
        const std::string displayed = needsQuotes ? '"' + completion + '"' : completion;

        buffer = buffer.substr(0, wordStart) + displayed;
        std::cout << displayed << std::flush;
    };

    auto handleUp = [&]()
    {
        if (!history || history->empty()) return;
        if (historyIndex == history->size())
            savedInput = buffer;
        if (historyIndex > 0)
        {
            --historyIndex;
            ReplaceBuffer(buffer, (*history)[historyIndex]);
            wasTab = false;
        }
    };

    auto handleDown = [&]()
    {
        if (!history || historyIndex >= history->size()) return;
        ++historyIndex;
        const std::string next = (historyIndex == history->size())
            ? savedInput
            : (*history)[historyIndex];
        ReplaceBuffer(buffer, next);
        wasTab = false;
    };

    auto commitLine = [&]()
    {
        std::cout << '\n';
        if (history && !buffer.empty())
        {
            if (history->empty() || history->back() != buffer)
                history->push_back(buffer);
        }
    };

#ifdef _WIN32

    while (true)
    {
        const int ch = _getch();

        if (ch == '\r' || ch == '\n')
        {
            commitLine();
            return buffer;
        }

        if (ch == '\b')
        {
            wasTab = false;
            if (!buffer.empty())
            {
                buffer.pop_back();
                std::cout << "\b \b" << std::flush;
            }
            continue;
        }

        if (ch == '\t') { handleTab(); continue; }

        if (ch == 0 || ch == 0xE0)
        {
            const int key = _getch();
            if      (key == 0x48) handleUp();   // up arrow
            else if (key == 0x50) handleDown();  // down arrow
            continue;
        }

        if (ch >= 32 && ch < 127)
        {
            wasTab = false;
            historyIndex = history ? history->size() : 0;
            buffer += static_cast<char>(ch);
            std::cout << static_cast<char>(ch) << std::flush;
        }
    }

#else

    // Unix: raw terminal mode — character-by-character, no line buffering, no echo
    termios oldt{};
    tcgetattr(STDIN_FILENO, &oldt);
    {
        termios newt = oldt;
        newt.c_lflag &= ~static_cast<tcflag_t>(ICANON | ECHO);
        newt.c_cc[VMIN]  = 1;
        newt.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }

    while (true)
    {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) != 1) break;

        const int ch = static_cast<unsigned char>(c);

        if (ch == '\r' || ch == '\n')
        {
            commitLine();
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            return buffer;
        }

        // Backspace: DEL (0x7f) or BS (0x08)
        if (ch == 0x7f || ch == '\b')
        {
            wasTab = false;
            if (!buffer.empty())
            {
                buffer.pop_back();
                std::cout << "\b \b" << std::flush;
            }
            continue;
        }

        if (ch == '\t') { handleTab(); continue; }

        // ESC sequence: arrow keys are ESC [ A/B/C/D
        if (ch == 0x1b)
        {
            char seq[2] = {};
            if (read(STDIN_FILENO, &seq[0], 1) != 1 || seq[0] != '[') continue;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) continue;

            if      (seq[1] == 'A') handleUp();
            else if (seq[1] == 'B') handleDown();
            continue;
        }

        if (ch >= 32 && ch < 127)
        {
            wasTab = false;
            historyIndex = history ? history->size() : 0;
            buffer += static_cast<char>(ch);
            std::cout << static_cast<char>(ch) << std::flush;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return buffer;

#endif
}
