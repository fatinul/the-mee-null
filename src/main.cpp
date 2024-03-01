#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include <memory> // for allocator, __shared_ptr_access
#include <string> // for char_traits, operator+, string, basic_string

#include "ftxui/component/captured_mouse.hpp"     // for ftxui
#include "ftxui/component/component.hpp"          // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp" // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"                 // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"                     // for Ref

class TheMeeNullMarkers {
    public:
        static const std::string themeenull_start;
        static const std::string themeenull_end;
        bool inTargetSection;
};

const std::string TheMeeNullMarkers::themeenull_start = "#################### THE-MEE-NULL ####################";
const std::string TheMeeNullMarkers::themeenull_end = "######################### END ########################";

void deleteTheMeeNullMarkers(std::string bashrc_path)
{
    std::ifstream inFile(bashrc_path);
    std::ofstream outFile(bashrc_path + ".tmp");
    std::string line;
    bool deleting = false;

    if (inFile.is_open() && outFile.is_open()) {
        while (std::getline(inFile, line)) {
            if (line.find(TheMeeNullMarkers::themeenull_start) != std::string::npos) {
                deleting = true;
            } else if (line.find(TheMeeNullMarkers::themeenull_end) != std::string::npos) {
                deleting = false;
            } else if (!deleting) {
                outFile << line << "\n";
            }
        }
        inFile.close();
        outFile.close();
        std::remove(bashrc_path.c_str());
        std::rename((bashrc_path + ".tmp").c_str(), bashrc_path.c_str());
    }
}

int main()
{
    using namespace ftxui;
    auto screen = ScreenInteractive::FitComponent();

    // Markers
    TheMeeNullMarkers markers;

    // The data:
    std::string welcome_text;
    std::string welcome_font;
    std::string bashrc_content;
    std::string modified_bashrc_content;
    std::string log;

    const char* home_dir = getenv("HOME");
    if(home_dir == nullptr){
        log = "Error: Unable to get home directory";
        screen.ExitLoopClosure();
    }

    // .bashrc path
    std::string bashrc_path = std::string(home_dir) + "/.bashrc";

    // The save button action:
    auto on_save_button = [&] {
        deleteTheMeeNullMarkers(bashrc_path);

        std::ofstream outFile(bashrc_path, std::ios::app);
        if(outFile.is_open()){
            // Write the modified content to .bashrc file:
            outFile << TheMeeNullMarkers::themeenull_start << "\n" << modified_bashrc_content << "\n" << TheMeeNullMarkers::themeenull_end << "\n";
            outFile.close();
            log = "Successful!";
        } else {
            log = "Error";
            screen.ExitLoopClosure();
        }
    };

    // The basic input components:
    auto input_welcome_text = Input(&welcome_text, "Type text you want when first open terminal here");
    auto input_welcome_font = Input(&welcome_font, "Type name of font you want for welcoming message here");
    auto button_cancel = Button("Cancel", screen.ExitLoopClosure());
    auto button_save = Button("Save", on_save_button);

    // The component tree:
    auto component = Container::Vertical({
        input_welcome_text,
        input_welcome_font,
        button_cancel,
        button_save,
    });

    // Tweak how the component tree is rendered:
    auto renderer = Renderer(component, [&]
                             {
                                // Modify the .bashrc file:
                                modified_bashrc_content = "\necho \"\"\necho \"\"\nfiglet -f " + welcome_font + " \"" + welcome_text + "\"\n";
                                
                                return vbox({
                                        text("Welcome to THE-MEE-NULL") | border,
                                        hbox(text(" Text : "), input_welcome_text->Render()),
                                        hbox(text(" Font name  : "), input_welcome_font->Render()),
                                        hbox(button_cancel->Render(),button_save->Render()),
                                        separator(),
                                        hbox(text("log : "),vbox(text(log))),
                                        hbox(text("command append : figlet -f " + welcome_font + " \"" + welcome_text + "\"")),
                                    }) |
                                    border; });

    screen.Loop(renderer);
}

/* TODO
## Welcome Text
- For now, it only append to the .bashrc file. Need to replace the figlet instead of just append.
- option for lolcat (toggle)
- list all the fonts available (menu)

## Quality of Life
- Make THE-MEE-NULL logo.. maybe change name lol
- Better UX

## ls, cd

## easier installation
*/