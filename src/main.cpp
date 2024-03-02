#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <filesystem>
#include <vector>
#include <algorithm>

#include <memory> // for allocator, __shared_ptr_access
#include <string> // for char_traits, operator+, string, basic_string

#include "ftxui/component/captured_mouse.hpp"     // for ftxui
#include "ftxui/component/component.hpp"          // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp" // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"                 // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"                     // for Ref

auto logo() {
    using namespace ftxui;

    return vbox({
       text(R"(                         ╔─╶╶╶╶╶╶╶╶ ──╗)"), 
       text(R"(                        ╔╝      ░░░░░▦│)"), 
       text(R"(                      ╒─╝          ░░▮╚╗)"), 
       text(R"(                      ▌▌ ▂ ▂▂▂▂▂▂    ░░│)"), 
       text(R"(                      ╘╍▂▂▂▂▂▂▂▂ ▓   ░═▀▀╗)"), 
       text(R"(                       │░░╿ ╭╯░░░    ▊   │)"), 
       text(R"(                      ╔─╝▗▘ ▞        ▊░  ╚╗)"), 
       text(R"(                      ▌│ ▌  ░   ░░░  ▊░  ░│)"), 
       text(R"(                      ▝▖░▌   ▔▲▒  ▒▒▒▊▒░ ▲│)"), 
       text(R"(                      ╔▌ ▝▀▀▀▀▀   ▒░  ▊▒ ╔╝)"), 
       text(R"(                      ▌ ▃▃▃▃▃▃   ░░   ╔──╝)"), 
       text(R"(                     ▗▘▀▒░░░░░▀ ░░   ░│)"), 
       text(R"(                     ▌│▂▂     ▂▂▂│ ░░░│)"), 
       text(R"(                     ▝▀───────────────╝)"), 
       text(R"(                - .... . -- . . -. ..- .-.. .-..)"), 
       text(R"( _____ _  _ ___         __  __ ___ ___         _  _ _   _ _    _)"), 
       text(R"(|_   _| || | __|  ___  |  \/  | __| __|  ___  | \| | | | | |  | |)"), 
       text(R"(  | | | __ | _|  |___| | |\/| | _|| _|  |___| | .` | |_| | |__| |__)"), 
       text(R"(  |_| |_||_|___|       |_|  |_|___|___|       |_|\_|\___/|____|____|)"), 
    });
}

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
    auto screen = ScreenInteractive::TerminalOutput();

    // Markers
    TheMeeNullMarkers markers;

    // The data:
    std::string welcome_text;
    std::string welcome_font;
    std::vector <std::string> welcome_font_entries;
    std::string bashrc_content;
    std::string modified_bashrc_content;
    std::string log;
    std::string lolcat_status;
    std::string ls = "ls";
    std::string ls_compact_status;
    std::string ls_color_status;
    std::string cd = "cd";

    int lolcat_selected = 0;
    int welcome_font_selected = 0;
    int ls_compact_selected = 0;
    int ls_color_selected = 0;
    int cd_autols_selected = 0;
    int cd_header_selected = 0;
    int cd_header_font_selected = 0;
    int cd_header_lolcat_selected = 0;

    // lolcat entries
    std::vector<std::string> lolcat_entries = {
        "Enabled",
        "Disabled",
    };

    // ls_compact entries
    std::vector<std::string> ls_compact_entries = {
        "Compact",
        "Simple",
    };

    // ls_color entries
    std::vector<std::string> ls_color_entries = {
        "Enabled",
        "Disabled",
    };

    // cd_autols entries
    std::vector<std::string> cd_autols_entries = {
        "Enabled",
        "Disabled",
    };

    // cd_autols entries
    std::vector<std::string> cd_header_entries = {
        "Enabled",
        "Disabled",
    };

    // Get the font entries
    for (const auto& entry : std::filesystem::directory_iterator("/usr/share/figlet")) {
        if(entry.path().extension() == ".tlf" || entry.path().extension() == ".flf"){
            welcome_font_entries.push_back(entry.path().filename().string());
        }
    }

    // Sort the font entries
    std::sort(welcome_font_entries.begin(), welcome_font_entries.end());

    // Get the home directory
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
    auto input_welcome_text = Input(&welcome_text, "Here..");
    auto button_cancel = Button("Cancel [ctrl + c]", screen.ExitLoopClosure());
    auto button_save = Button("Save", on_save_button);
    auto toggle_lolcat = Toggle(&lolcat_entries, &lolcat_selected);
    auto menu_welcome_font = Menu(&welcome_font_entries, &welcome_font_selected);
    auto toggle_ls_compact = Toggle(&ls_compact_entries, &ls_compact_selected);
    auto toggle_ls_color = Toggle(&ls_color_entries, &ls_color_selected);
    auto toggle_cd_autols = Toggle(&cd_autols_entries, &cd_autols_selected);
    auto toggle_cd_header = Toggle(&cd_header_entries, &cd_header_selected);

    // The component tree:
    auto component = Container::Vertical({
        input_welcome_text,
        button_cancel,
        button_save,
        toggle_lolcat,
        menu_welcome_font,
        toggle_ls_compact,
        toggle_ls_color,
        toggle_cd_autols,
        toggle_cd_header,
    });

    // Tweak how the component tree is rendered:
    auto renderer = Renderer(component, [&]
                             {
                                // Get the selected font name
                                welcome_font = welcome_font_entries[welcome_font_selected];

                                // Modify cd_func


                                // Modify the .bashrc file:
                                modified_bashrc_content = "\necho \"\"\necho \"\"\nfiglet -f '" + welcome_font + "' \"" + welcome_text + "\"" + lolcat_status +
                                 "\nalias ls='" + ls + 
                                 "'\nalias cd='cd_func'\ncd_func(){\n\t" + cd + "\n}\n";

                                // toggle lolcat
                                if(!lolcat_selected){
                                    lolcat_status = " | lolcat";
                                } else {
                                    lolcat_status = "";
                                }

                                // toggle ls compact & ls_color
                                if(!ls_compact_selected){
                                    // toggle ls color
                                    if(!ls_color_selected){
                                        ls = "ls -lh --color=auto";
                                    } else {
                                        ls = "ls -lh";
                                    }
                                } else {
                                    // toggle ls color
                                    if(!ls_color_selected){
                                        ls = "ls --color=auto";
                                    } else {
                                        ls = "ls";
                                    }
                                }

                                // toggle cd_header
                                if(!cd_header_selected){
                                    // toggle cd_autols
                                    if(!cd_autols_selected){
                                        cd = "builtin cd \"$@\" && printf \"%s\n\" \"$(pwd | rev | cut -d'/' -f1 | rev)\" | figlet -f future | lolcat && ls";
                                    } else {
                                        cd = "builtin cd \"$@\" && printf \"%s\n\" \"$(pwd | rev | cut -d'/' -f1 | rev)\" | figlet -f future | lolcat";
                                    }
                                } else {
                                    // toggle cd_autols
                                    if(!cd_autols_selected){
                                        cd = "builtin cd \"$@\" && ls";
                                    } else {
                                        cd = "builtin cd \"$@\"";
                                    }
                                }
                                
                                // window frame
                                return vbox({
                                        hbox(filler(), button_cancel->Render()),
                                        hbox({
                                            filler(), logo(), filler(),
                                        }),
                                        text(""),
                                        hbox({
                                            // welcome text
                                            window(text("Welcome text"), 
                                                vbox({
                                                    hbox(text(" Text    : "), input_welcome_text->Render()),
                                                    hbox(text(" lolcat  : "), toggle_lolcat->Render()),
                                                    hbox(text(" Font    : "), menu_welcome_font->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10) | border),
                                                })
                                            ),
                                            // command
                                            window(text("Command"), 
                                                vbox({
                                                    window(text("ls"),vbox({
                                                       hbox(text(" List  : "), toggle_ls_compact->Render()), 
                                                       hbox(text(" Color  : "), toggle_ls_color->Render()), 
                                                    })),
                                                    window(text("cd"),vbox({
                                                       hbox(text(" Header  : "), toggle_cd_header->Render()), 
                                                       hbox(text(" Auto List  : "), toggle_cd_autols->Render()), 
                                                    })),
                                                })
                                            ),
                                        }),
                                       button_save->Render(),
                                        separator(),
                                        hbox(text("log : "),vbox(text(log))),
                                        hbox(text("command append : ")),
                                        hbox(text("  * figlet -f '" + welcome_font + "' \"" + welcome_text + "\"" + lolcat_status)),
                                        hbox(text("  * alias ls=' " + ls + "'")),
                                        hbox(text("  * alias cd=' " + cd + "'")),
                                    }) |
                                    border; });

    screen.Loop(renderer);
}

/* TODO
## Quality of Life
- Better UX
---- color

## cd, PS1
## easier installation
## screen adapt
*/