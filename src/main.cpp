#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <csignal>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <sys/ioctl.h>
#include <sys/unistd.h>

#include <memory> // for allocator, __shared_ptr_access
#include <string> // for char_traits, operator+, string, basic_string

#include "ftxui/component/captured_mouse.hpp"     // for ftxui
#include "ftxui/component/component.hpp"          // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp" // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"                 // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"                     // for Ref
#include "ftxui/screen/color.hpp"                 // for Color
#include "ftxui/dom/node.hpp"                     // for Render
#include "ftxui/screen/terminal.hpp"              // for TerminalOutput

auto logo() {
    using namespace ftxui;
    
    return vbox({
        text(R"(  ╓╌╌─╗              - .... . -- . . -. ..- .-.. .-..)"), 
        text(R"( ╔▁▁▁░╚╗ |"|_|"|_ ___    ___    _____ ___ ___    ___   ___ _ _|"|"|)"), 
        text(R"( ╛▟▲▘ ▒║ |  _|   | -_|  |___|  |     | -_| -_|  |___| |   | | | | |)"), 
        text(R"(╭╒╼╾╮░╓╜ |_| |_|_|___|         |_|_|_|___|___|        |_|_|___|_|_|)"), 
        text(R"(╚─────╝)"), 
    });

    // return vbox({
    //    text(R"(                         ╔─╶╶╶╶╶╶╶╶ ──╗)"), 
    //    text(R"(                        ╔╝      ░░░░░▦│)"), 
    //    text(R"(                      ╒─╝          ░░▮╚╗)"), 
    //    text(R"(                      ▌▌ ▂ ▂▂▂▂▂▂    ░░│)"), 
    //    text(R"(                      ╘╍▂▂▂▂▂▂▂▂ ▓   ░═▀▀╗)"), 
    //    text(R"(                       │░░╿ ╭╯░░░    ▊   │)"), 
    //    text(R"(                      ╔─╝▗▘ ▞        ▊░  ╚╗)"), 
    //    text(R"(                      ▌│ ▌  ░   ░░░  ▊░  ░│)"), 
    //    text(R"(                      ▝▖░▌   ▔▲▒  ▒▒▒▊▒░ ▲│)"), 
    //    text(R"(                      ╔▌ ▝▀▀▀▀▀   ▒░  ▊▒ ╔╝)"), 
    //    text(R"(                      ▌ ▃▃▃▃▃▃   ░░   ╔──╝)"), 
    //    text(R"(                     ▗▘▀▒░░░░░▀ ░░   ░│)"), 
    //    text(R"(                     ▌│▂▂     ▂▂▂│ ░░░│)"), 
    //    text(R"(                     ▝▀───────────────╝)"), 
    //    text(R"(                - .... . -- . . -. ..- .-.. .-..)"), 
    //    text(R"( _____ _  _ ___         __  __ ___ ___         _  _ _   _ _    _)"), 
    //    text(R"(|_   _| || | __|  ___  |  \/  | __| __|  ___  | \| | | | | |  | |)"), 
    //    text(R"(  | | | __ | _|  |___| | |\/| | _|| _|  |___| | .` | |_| | |__| |__)"), 
    //    text(R"(  |_| |_||_|___|       |_|  |_|___|___|       |_|\_|\___/|____|____|)"), 
    // });
}

class TheMeeNullMarkers {
    public:
        static const std::string themeenull_start;
        static const std::string themeenull_end;
        bool inTargetSection;
};

// Assign const
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

int getTerminalHeight() {
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    return size.ws_row;
}
int getTerminalWidth() {
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    return size.ws_col;
}

ftxui::Element ColorString(int red, int green, int blue){
    return ftxui::text("RGB = (" + std::to_string(red) + ", " + std::to_string(green) + ", " + std::to_string(blue) + ")");
}

std::string PS1ColorString(int red, int green, int blue){
    return std::to_string(red) + ";" + std::to_string(green) + ";" + std::to_string(blue);
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
    std::string cd_header_font;
    std::string cd_header_lolcat_status;
    std::vector <std::string> cd_header_font_entries;
    std::string PS1;

    // const func for git
    const std::string extract_git_script = "parse_git_branch() {\n\tgit branch 2> /dev/null | sed -e '/^[^*]/d' -e 's/* \\(.*\\)/  \\1 /'\n}";

    int lolcat_selected = 0;
    int welcome_font_selected = 0;
    int ls_compact_selected = 0;
    int ls_color_selected = 0;
    int cd_autols_selected = 0;
    int cd_header_selected = 0;
    int cd_header_font_selected = 0;
    int cd_header_lolcat_selected = 0;
    int ps1_tab_selected = 0;
    int ps1_hostname_selected = 0;
    int ps1_path_selected = 0;
    int ps1_git_selected = 0;
    int ps1_command_selected = 0;
    int screen_height = 0;
    int screen_width = 0;

    // modal layer
    int depth = 0;

    // screen small bool
    bool screen_small = false;

    // RGB
    int ps1_hostname_red = 70;
    int ps1_hostname_green = 141;
    int ps1_hostname_blue = 127;
    int ps1_path_red = 36;
    int ps1_path_green = 39;
    int ps1_path_blue = 58;
    int ps1_git_red = 70;
    int ps1_git_green = 141;
    int ps1_git_blue = 127;
    int ps1_command_red = 36;
    int ps1_command_green = 39;
    int ps1_command_blue = 58;

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

    // cd_lolcat entries
    std::vector<std::string> cd_header_lolcat_entries = {
        "Enabled",
        "Disabled",
    };

    // ps1_tab entries
    std::vector<std::string> ps1_tab_entries = {
        "Hostname",
        "Path",
        "Git",
        "Command",
    };

    // Get the font entries
    for (const auto& entry : std::filesystem::directory_iterator("/usr/share/figlet")) {
        if(entry.path().extension() == ".tlf" || entry.path().extension() == ".flf"){
            welcome_font_entries.push_back(entry.path().filename().string());
            cd_header_font_entries.push_back(entry.path().filename().string());
        }
    }

    // Sort the font entries
    std::sort(welcome_font_entries.begin(), welcome_font_entries.end());
    std::sort(cd_header_font_entries.begin(), cd_header_font_entries.end());

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

        depth = 2;
    };

    // screen small to depth
    if(screen_small){
        depth = 3;
    } else {
        depth = 0;
    }

    // The basic input components:
    auto input_welcome_text = Input(&welcome_text, "Here..");
    auto button_quit = Button("     Quit [ctrl + c]", [&] { depth = 1; } );
    auto button_save = Button("                                           Save ", on_save_button);
    auto toggle_lolcat = Toggle(&lolcat_entries, &lolcat_selected);
    auto menu_welcome_font = Menu(&welcome_font_entries, &welcome_font_selected);
    auto toggle_ls_compact = Toggle(&ls_compact_entries, &ls_compact_selected);
    auto toggle_ls_color = Toggle(&ls_color_entries, &ls_color_selected);
    auto toggle_cd_autols = Toggle(&cd_autols_entries, &cd_autols_selected);
    auto toggle_cd_header = Toggle(&cd_header_entries, &cd_header_selected);
    auto menu_cd_header_font = Menu(&cd_header_font_entries, &cd_header_font_selected);
    auto toggle_cd_header_lolcat = Toggle(&cd_header_lolcat_entries, &cd_header_lolcat_selected);
    auto menu_ps1_tab = Menu(&ps1_tab_entries, &ps1_tab_selected);
    auto slider_ps1_hostname_red = Slider("R:", &ps1_hostname_red, 0, 255, 1);
    auto slider_ps1_hostname_green = Slider("G:", &ps1_hostname_green, 0, 255, 1);
    auto slider_ps1_hostname_blue = Slider("B:", &ps1_hostname_blue, 0, 255, 1);
    auto slider_ps1_path_red = Slider("R:", &ps1_path_red, 0, 255, 1);
    auto slider_ps1_path_green = Slider("G:", &ps1_path_green, 0, 255, 1);
    auto slider_ps1_path_blue = Slider("B:", &ps1_path_blue, 0, 255, 1);
    auto slider_ps1_git_red = Slider("R:", &ps1_git_red, 0, 255, 1);
    auto slider_ps1_git_green = Slider("G:", &ps1_git_green, 0, 255, 1);
    auto slider_ps1_git_blue = Slider("B:", &ps1_git_blue, 0, 255, 1);
    auto slider_ps1_command_red = Slider("R:", &ps1_command_red, 0, 255, 1);
    auto slider_ps1_command_green = Slider("G:", &ps1_command_green, 0, 255, 1);
    auto slider_ps1_command_blue = Slider("B:", &ps1_command_blue, 0, 255, 1);

    // container_ps1_hostname
    auto container_ps1_hostname_rgb = Container::Vertical({
        slider_ps1_hostname_red,
        slider_ps1_hostname_green,
        slider_ps1_hostname_blue,
    });

    // container_ps1_path
    auto container_ps1_path_rgb = Container::Vertical({
        slider_ps1_path_red,
        slider_ps1_path_green,
        slider_ps1_path_blue,
    });

    // container_ps1_git
    auto container_ps1_git_rgb = Container::Vertical({
        slider_ps1_git_red,
        slider_ps1_git_green,
        slider_ps1_git_blue,
    });

    // container_ps1_command
    auto container_ps1_command_rgb = Container::Vertical({
        slider_ps1_command_red,
        slider_ps1_command_green,
        slider_ps1_command_blue,
    });

    // ps1 tab container
    auto container_ps1_tab = Container::Tab({ 
        container_ps1_hostname_rgb,
        container_ps1_path_rgb,
        container_ps1_git_rgb,
        container_ps1_command_rgb,
    }, &ps1_tab_selected);

    // The component tree:
    auto component = Container::Vertical({
        input_welcome_text,
        toggle_lolcat,
        menu_welcome_font,
        toggle_ls_compact,
        toggle_ls_color,
        toggle_cd_header,
        toggle_cd_autols,
        toggle_cd_header_lolcat,
        menu_cd_header_font,
        menu_ps1_tab,
        container_ps1_tab,
        button_save,
        button_quit,
    });

    // quit
    auto quit_dialog_container = Container::Horizontal({
        Button("   No   ", [&] { depth = 0; }),
        Button("   Yes  ", [&] { kill(getpid(), SIGINT); }),
    });

    // saved_dialog_container
    auto saved_dialog_container = Container::Horizontal({
        Button("        Ok  󰩐      ", [&] { depth = 0; }),
    });

    // screen_size container
    auto screen_size_container = Container::Horizontal({
    });

    // Tweak how the component tree is rendered:
    auto renderer = Renderer(component, [&]
                             {
                                // Get the selected font name
                                welcome_font = welcome_font_entries[welcome_font_selected];
                                cd_header_font = cd_header_font_entries[cd_header_font_selected];

                                // Modify the .bashrc file:
                                modified_bashrc_content = "\necho \"\"\nfiglet -f '" + welcome_font + "' \"" + welcome_text + "\"" + lolcat_status +
                                 "\nalias ls='" + ls + 
                                 "'\nalias cd='cd_func'\ncd_func(){\n\t" + cd + "\n}\n" + 
                                 extract_git_script + "\n" + PS1;

                                // toggle lolcat welcome
                                if(!lolcat_selected){
                                    lolcat_status = " | lolcat";
                                } else {
                                    lolcat_status = "";
                                }

                                // toggle lolcat cd_header
                                if(!cd_header_lolcat_selected){
                                    cd_header_lolcat_status = "' | lolcat";
                                } else {
                                    cd_header_lolcat_status = "'";
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
                                        cd = "builtin cd \"$@\" && printf \"%s\" \"$(pwd | rev | cut -d'/' -f1 | rev)\" | figlet -f '" + cd_header_font + cd_header_lolcat_status +"&& ls";
                                    } else {
                                        cd = "builtin cd \"$@\" && printf \"%s\" \"$(pwd | rev | cut -d'/' -f1 | rev)\" | figlet -f '" + cd_header_font + cd_header_lolcat_status;
                                    }
                                } else {
                                    // toggle cd_autols
                                    if(!cd_autols_selected){
                                        cd = "builtin cd \"$@\" && ls";
                                    } else {
                                        cd = "builtin cd \"$@\"";
                                    }
                                }

                                // Check screen size
                                screen_height = getTerminalHeight();
                                screen_width = getTerminalWidth();

                                if(screen_height < 30 || screen_width < 106){
                                    screen_small = true;
                                } else {
                                    screen_small = false;
                                }

                                // modify PS1
                                PS1 = "PS1='\\[\\033[48;2;" + PS1ColorString(ps1_hostname_red, ps1_hostname_green, ps1_hostname_blue) + 
                                ";38;2;255;255;255m\\] \\u@\\h \\[\\033[48;2;" + PS1ColorString(ps1_path_red, ps1_path_green, ps1_path_blue) + 
                                ";38;2;" + PS1ColorString(ps1_hostname_red, ps1_hostname_green, ps1_hostname_blue) + 
                                "m\\]\\[\\033[48;2;" + PS1ColorString(ps1_path_red, ps1_path_green, ps1_path_blue) +
                                ";38;2;255;255;255m\\] \\w  \\[\\033[49;38;2;" + PS1ColorString(ps1_path_red, ps1_path_green, ps1_path_blue) +
                                "m\\] \\[\\033[38;2;" + PS1ColorString(ps1_git_red, ps1_git_green, ps1_git_blue) + 
                                "m\\]$(parse_git_branch)\\n\\[\\033[48;2;" + PS1ColorString(ps1_command_red, ps1_command_green, ps1_command_blue) + 
                                ";38;2;255;255;255m\\] \\$ \\[\\033[49;38;2;" + PS1ColorString(ps1_command_red, ps1_command_green, ps1_command_blue) + 
                                "m\\]\\[\\033[00m\\] '";

                                // window frame
                                return vbox({
                                        color(Color::RGB(105,105,105),
                                        hbox(filler(), button_quit->Render())),
                                        hbox({
                                            filler(), color(Color::RGB(101,83,83), logo()), filler(),
                                        }),
                                        text(""),
                                        hbox({
                                            // welcome text
                                            color((Color::RGB(237,184,139)),
                                            window(text("Welcome text 󱠡 "), 
                                                vbox({
                                                    hbox(text(" Text    : "), input_welcome_text->Render()),
                                                    hbox(text(" Lolcat  : "), toggle_lolcat->Render()),
                                                    hbox(text(" Font    : "), menu_welcome_font->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10) | border),
                                                }))
                                            ),
                                            // command
                                            vbox({
                                                color((Color::RGB(110,136,148)),
                                                window(text("ls  "),vbox({
                                                    hbox(text(" List    : "), toggle_ls_compact->Render()), 
                                                    hbox(text(" Color   : "), toggle_ls_color->Render()), 
                                                }))),
                                                color(Color::RGB(153,88,42),
                                                window(text("cd  "),vbox({
                                                    hbox(text(" Header     : "), toggle_cd_header->Render()), 
                                                    hbox(text(" Auto List  : "), toggle_cd_autols->Render()), 
                                                    hbox(text(" Lolcat     : "), toggle_cd_header_lolcat->Render()),
                                                    hbox(text(" Font       : "), menu_cd_header_font->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 5) | border),
                                                }))),
                                            }),
                                            // PS1
                                            window(text("PS1 󰸻 "), 
                                                vbox({
                                                    hbox({
                                                        bgcolor(Color::RGB(ps1_hostname_red, ps1_hostname_green, ps1_hostname_blue), text(" name@hostname ")),
                                                        bgcolor(Color::RGB(ps1_path_red, ps1_path_green, ps1_path_blue), color(Color::RGB(ps1_hostname_red, ps1_hostname_green, ps1_hostname_blue), text(""))),
                                                        bgcolor(Color::RGB(ps1_path_red, ps1_path_green, ps1_path_blue), color(Color::White, text(" the/path "))),
                                                        color(Color::RGB(ps1_path_red, ps1_path_green, ps1_path_blue), text("")),
                                                        color(Color::RGB(ps1_git_red, ps1_git_green, ps1_git_blue), text("   git ")),
                                                    }),
                                                    hbox({
                                                        bgcolor(Color::RGB(ps1_command_red, ps1_command_green, ps1_command_blue), text(" $ ")),
                                                        color(Color::RGB(ps1_command_red, ps1_command_green, ps1_command_blue), text("")),
                                                        text("    "),
                                                    }),
                                                    filler(),
                                                    hbox({
                                                       text(" Hostname : "), 
                                                       ColorString(ps1_hostname_red, ps1_hostname_green, ps1_hostname_blue),
                                                    }),
                                                    hbox({
                                                       text(" Path     : "), 
                                                       ColorString(ps1_path_red, ps1_path_green, ps1_path_blue),
                                                    }),
                                                    hbox({
                                                       text(" Git      : "), 
                                                       ColorString(ps1_git_red, ps1_git_green, ps1_git_blue),
                                                    }),
                                                    hbox({
                                                       text(" Command  : "), 
                                                       ColorString(ps1_command_red, ps1_command_green, ps1_command_blue),
                                                    }),
                                                    filler(),
                                                    hbox({
                                                        menu_ps1_tab->Render(),
                                                        separator(),
                                                        container_ps1_tab->Render() | flex,
                                                    }) | border,
                                                })
                                            ),
                                        }),
                                        hbox({
                                            color(Color::RGB(255,255,255),text("(H: " + std::to_string(getTerminalHeight()) + ", W: " + std::to_string(getTerminalWidth()) + ")")) | border, 
                                            color(Color::RGB(156,175,136),button_save->Render()) | flex ,
                                        }),
                                    }) |
                                    border; });
    
    // quit dialog renderer
    auto quit_renderer = Renderer(quit_dialog_container, [&] {
       return vbox({
            text("Are you sure you want to quit?"),
            separator(),
            hbox({filler(), quit_dialog_container->Render(), filler()}),
       }); 
    });

    // saved dialog renderer
    auto saved_renderer = Renderer(saved_dialog_container, [&] {
       return color(Color::RGB(156,175,136),vbox({
            text("                    SAVED!"),
            text("Open your terminal and see your masterpiece!"),
            separator(),
            hbox({filler(), saved_dialog_container->Render(), filler()}),
       })); 
    });

    

    auto main_container = Container::Tab({
        renderer,
        quit_renderer,
        saved_renderer,
    }, &depth);

    auto main_renderer = Renderer(main_container, [&] {
        Element document = renderer->Render();

        if (depth == 1){
            document = dbox({
               document,
               quit_renderer->Render() | borderDouble | clear_under | center, 
            });
        } else if (depth == 2){
            document = dbox({
                document,
                saved_renderer->Render() | borderDouble | clear_under | center,
            });
        } 
        return document;
    });

    screen.Loop(main_renderer);
    return 0;
}

/* TODO
## easier installation
---- edit cmake
---- edit tell if no figlet and lolcat

## Readme
---- explain installation and gif

## Release package
---- cmake
*/