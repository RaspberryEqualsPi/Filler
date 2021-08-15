// if you're having issues with git, remember: commit all, then push, it works every time
// TODO: make create ghost button create empty ghost
#include <map>
#include <fstream>
#include <iostream>
#include "buttonHover.h"
#include "Collision.h"
#include "FillerUI.h"
#include <SFML/Audio.hpp>
#include "ObjectTypes.h"
#include "dirent.h"
#include "Timer.h"
#include "rapidjson/document.h"
using namespace rapidjson;
using namespace std;
int levels = 0;
bool up1;
bool down1;
bool right1;
bool left1;
bool lost = false;
bool won = true;
bool ghostWon = false;
sf::Vector2i startingpos;
sf::Texture wstex;
sf::Sprite winscreen;
sf::Texture ltex;
sf::Sprite losescreen;
sf::Texture estex;
sf::Sprite endscreen;
std::string lastfilename;
std::map<int, Token> tokens;
std::map<int, ArrowShooter> ArrowShooters;
std::map<int, Checkpoint> Checkpoints;
std::map<int, Token> gTokens;
std::map<int, ArrowShooter> gArrowShooters;
std::map<int, Checkpoint> gCheckpoints;
std::map<int, Token> LastTokenState;
int LastTokenStateLevel;
Player* lastPlayerState = nullptr;
Player player(0,0);
sf::RenderWindow window(sf::VideoMode(320, 320), "Filler");
bool isNumber(const string& str)
{
    for (char const& c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}
string getexepath()
{
    char szFilePath[MAX_PATH + 1] = { 0 };
    GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
    (strrchr(szFilePath, '\\'))[0] = 0;
    string path = szFilePath;
    return path;
}
std::string process(std::string const& s, std::string delimiter)
{
    std::string::size_type pos = s.find(delimiter);
    if (pos != std::string::npos)
    {
        return s.substr(0, pos);
    }
    else
    {
        return s;
    }
}
void loadlevel(std::string filename) {
    string json;
    string line;
    ifstream f(filename);
    if (f.is_open())
    {
        while (getline(f, line))
        {
            json = json + line;
        }
        f.close();
    }
    Document level;
    level.Parse(json.c_str());
    startingpos = { level["StartingX"].GetInt(), level["StartingY"].GetInt() };
    player = Player(level["StartingX"].GetInt(), level["StartingY"].GetInt());
    if (level.HasMember("Tokens")) {
        for (size_t i = 0; i < level["Tokens"].Size(); i++) {
            const Value& token1 = level["Tokens"][i];
            Token token;
            token.x = token1["x"].GetInt();
            token.y = token1["y"].GetInt();
            tokens.insert({ i, token });
        }
    }
    if (level.HasMember("ArrowShooters")) {
        for (size_t i = 0; i < level["ArrowShooters"].Size(); i++) {
            const Value& Arrowshooter1 = level["ArrowShooters"][i];
            ArrowShooter Arrowshooter;
            Arrowshooter.x = Arrowshooter1["x"].GetInt();
            Arrowshooter.y = Arrowshooter1["y"].GetInt();
            Arrowshooter.type = Arrowshooter1["type"].GetInt();
            ArrowShooters.insert({ i, Arrowshooter });
        }
    }
    if (level.HasMember("Checkpoints")) {
        for (size_t i = 0; i < level["Checkpoints"].Size(); i++) {
            const Value& Checkpoint1 = level["Checkpoints"][i];
            Checkpoint Checkpoint;
            Checkpoint.x = Checkpoint1["x"].GetInt();
            Checkpoint.y = Checkpoint1["y"].GetInt();
            Checkpoints.insert({ i, Checkpoint });
        }
    }
}
int singlePlayerMain()
{
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);
    loadlevel("data/levels/level" + to_string(level) + ".json");
    wstex.loadFromFile("data/winscreen.png");
    winscreen.setTexture(wstex);
    ltex.loadFromFile("data/losescreen.png");
    losescreen.setTexture(ltex);
    estex.loadFromFile("data/endscreen_ORISIT.png");
    endscreen.setTexture(estex);
    losescreen.setPosition(0, -48);
    winscreen.setPosition(0, -48);
    endscreen.setPosition(0, -48);
    FillerUI::TextButton nextLvlB;
    nextLvlB.text = "Next Level";
    nextLvlB.rect.setPosition(50, 200);
    FillerUI::TextButton exitB;
    exitB.text = "Exit";
    exitB.rect.setPosition(320 - 16 - exitB.sizeX, 200);
    FillerUI::TextButton retryB;
    retryB.text = "Retry";
    retryB.rect.setPosition(16, 200);
    FillerUI::TextButton resetB;
    resetB.text = "Reset";
    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir("./data/levels")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            string name = string(ent->d_name);
            if (name.find("level") != std::string::npos && name.find(".json") != std::string::npos) {
                std::string beforeJson = process(name, ".json");
                std::string afterLevel = beforeJson.substr(beforeJson.find("level") + 5);
                if (isNumber(afterLevel)) {
                    levels++;
                }
            }
        }
        closedir(dir);
    }
    while (window.isOpen())
    {
        sf::Event event;
        for (size_t i = 0; i < tokens.size(); i++) {
            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                tokens[i].timestouched = 1;
                tokens[i].tokencolor = sf::Color::Blue;
            }
        }
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonReleased)
                FillerUI::SetClicking(true);
            if (event.type == sf::Event::KeyPressed) {
                if (!lost && !won) {
                    if (event.key.code == sf::Keyboard::E && appInFocus(&window)) {
                        sf::Vector2 lastpos = { player.x, player.y };
                        player.y--;
                        player.x++;
                        bool touched = false;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.y = lastpos.y;
                            player.x = lastpos.x;
                        }
                        break;
                    }
                    if (event.key.code == sf::Keyboard::D && appInFocus(&window)) {
                        sf::Vector2 lastpos = { player.x, player.y };
                        player.y++;
                        player.x++;
                        bool touched = false;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.y = lastpos.y;
                            player.x = lastpos.x;
                        }
                        break;
                    }
                    if (event.key.code == sf::Keyboard::W && appInFocus(&window)) {
                        sf::Vector2 lastpos = { player.x, player.y };
                        player.y--;
                        player.x--;
                        bool touched = false;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.y = lastpos.y;
                            player.x = lastpos.x;
                        }
                        break;
                    }
                    if (event.key.code == sf::Keyboard::S && appInFocus(&window)) {
                        sf::Vector2 lastpos = { player.x, player.y };
                        player.y++;
                        player.x--;
                        bool touched = false;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.y = lastpos.y;
                            player.x = lastpos.x;
                        }
                        break;
                    }
                    if (event.key.code == sf::Keyboard::Key::Down && appInFocus(&window)) {
                        int lastpos = player.y;
                        bool touched = false;
                        player.y++;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.y = lastpos;
                        }
                        break;
                    }
                    if (event.key.code == sf::Keyboard::Key::Up && appInFocus(&window)) {
                        int lastpos = player.y;
                        bool touched = false;
                        player.y--;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.y = lastpos;
                        }
                        break;
                    }
                    if (event.key.code == sf::Keyboard::Key::Left && appInFocus(&window)) {
                        int lastpos = player.x;
                        bool touched = false;
                        player.x--;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.x = lastpos;
                        }
                        break;
                    }
                    if (event.key.code == sf::Keyboard::Key::Right && appInFocus(&window)) {
                        int lastpos = player.x;
                        bool touched = false;
                        player.x++;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.x = lastpos;
                        }
                        break;
                    }
                }
            }
        }
        won = true;
        window.clear();
        for (size_t i = 0; i < tokens.size(); i++) {
            Token token = tokens[i];
            if (token.x == player.x && token.y == player.y) {
                if (!token.debounce) {
                    token.timestouched = token.timestouched + 1;
                    token.tokencolor = sf::Color::Blue;
                }
                token.debounce = true;
            }
            else {
                token.debounce = false;
            }
            tokens[i] = token;
            if (token.timestouched > 1)
                lost = true;
            token.draw(&window);
        }
        for (size_t i = 0; i < tokens.size(); i++) {
            if (tokens[i].timestouched == 0)
                won = false;
        }
        for (size_t i = 0; i < Checkpoints.size(); i++) {
            Checkpoints[i].draw(&window);
        }
        for (int i = 0; i < ArrowShooters.size(); i++) {
            /*if (ArrowShooters[i].type == 1)
                if (ArrowShooters[i].arrow.x == player.x && ArrowShooters[i].arrow.y <= player.y && !(ArrowShooters[i].arrow.y <= player.y - 1))
                    lost = true;
            if (ArrowShooters[i].type == 2)
                if (ArrowShooters[i].arrow.y == player.y && ArrowShooters[i].arrow.x >= player.x && !(ArrowShooters[i].arrow.x >= player.x + 1))
                    lost = true;
            if (ArrowShooters[i].type == 3)
                if (ArrowShooters[i].arrow.x == player.x && ArrowShooters[i].arrow.y >= player.y && !(ArrowShooters[i].arrow.y >= player.y + 1))
                    lost = true;
            if (ArrowShooters[i].type == 0) {
                if (ArrowShooters[i].arrow.y == player.y && ArrowShooters[i].arrow.x <= player.x && !(ArrowShooters[i].arrow.x <= player.x - 1))
                    lost = true;
            }*/
            sf::Texture cTex;
            cTex.loadFromFile("data/hmmm.png");
            sf::Sprite cSpr;
            cSpr.setTexture(cTex);
            cSpr.setPosition(player.player.getPosition());
            if (Collision::PixelPerfectTest(ArrowShooters[i].arrow.Sarrow, cSpr))
                lost = true;
            ArrowShooters[i].draw(&window);
            /*if (player.player.getGlobalBounds().contains(sf::Vector2f(ArrowShooters[i].arrow.x * 16, ArrowShooters[i].arrow.y * 16))) {
                lost = true;
            }*/
        }
        for (size_t i = 0; i < Checkpoints.size(); i++) {
            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                Checkpoints[i].timestouched++;
                if (Checkpoints[i].timestouched <= 1) {
                    for (int i1 = 0; i1 < Checkpoints.size(); i1++) {
                        if (i1 != i) {
                            Checkpoints[i1].timestouched = 0;
                        }
                    }
                    LastTokenState = tokens;
                    LastTokenStateLevel = level;
                    lastPlayerState = new Player(player.x, player.y);
                }
            }
        }
        player.draw(&window);
       // if (FillerUI::clicking && appInFocus(&window) && !lost && !won) // add this back for special win
            //won = !won;
        if (won) {
            bool nextlevel = level + 1 <= levels;
            exitB.rect.setPosition(320 - 50 - exitB.sizeX, 200);
            for (int i = 0; i < ArrowShooters.size(); i++) {
                ArrowShooters[i].stop = true;
            }
            if (nextlevel) {
                nextLvlB.draw(&window);
                window.draw(winscreen);
            }
            else {
                window.draw(endscreen);
                float Width1 = 320;
                float Width2 = exitB.rect.getGlobalBounds().width;
                float X2 = exitB.rect.getPosition().x;
                exitB.rect.setPosition(((Width1 - Width2) / 2) , 200);
            }

            exitB.draw(&window);
            if (exitB.clicked(&window))
                return 0;
            if (nextLvlB.clicked(&window) && nextlevel) {
                level++;
                std::cout << "data/levels/level" + to_string(level) + ".json" << std::endl;
                tokens = {};
                ArrowShooters = {};
                Checkpoints = {};
                loadlevel("data/levels/level" + to_string(level) + ".json");
                won = false;
            }
        }
        if (lost) {
            exitB.rect.setPosition(320 - 16 - exitB.sizeX, 200);
            window.draw(losescreen);
            for (int i = 0; i < ArrowShooters.size(); i++) {
                ArrowShooters[i].stop = true;
            }
            //float Width1 = 320;
            //float Width2 = exitB.rect.getGlobalBounds().width;
            //float X2 = exitB.rect.getPosition().x;
            //exitB.rect.setPosition(((Width1 - Width2) / 2) , 200);
            exitB.draw(&window);
            retryB.draw(&window);
            float Width1 = 320;
            float Width2 = resetB.rect.getGlobalBounds().width;
            float X2 = resetB.rect.getPosition().x;
            resetB.rect.setPosition(((Width1 - Width2) / 2), 200);
            resetB.draw(&window);
            if (exitB.clicked(&window))
                return 0;
            if (retryB.clicked(&window)) {
                if (level == LastTokenStateLevel) {
                    tokens = LastTokenState;
                    player = *lastPlayerState;
                    lost = false;
                    for (int i = 0; i < ArrowShooters.size(); i++) {
                        ArrowShooters[i].stop = false;
                    }
                    ArrowShooters = {};
                    string json;
                    string line;
                    ifstream f("data/levels/level" + to_string(level) + ".json");
                    if (f.is_open())
                    {
                        while (getline(f, line))
                        {
                            json = json + line;
                        }
                        f.close();
                    }
                    Document level1;
                    level1.Parse(json.c_str());
                    if (level1.HasMember("ArrowShooters")) {
                        for (size_t i = 0; i < level1["ArrowShooters"].Size(); i++) {
                            const Value& Arrowshooter1 = level1["ArrowShooters"][i];
                            ArrowShooter Arrowshooter;
                            Arrowshooter.x = Arrowshooter1["x"].GetInt();
                            Arrowshooter.y = Arrowshooter1["y"].GetInt();
                            Arrowshooter.type = Arrowshooter1["type"].GetInt();
                            ArrowShooters.insert({ i, Arrowshooter });
                        }
                    }
                }
                else {
                    LastTokenState = {};
                    lastPlayerState = nullptr;
                }
                if (lastPlayerState == nullptr) {
                    tokens = {};
                    ArrowShooters = {};
                    Checkpoints = {};
                    loadlevel("data/levels/level" + to_string(level) + ".json");
                    lost = false;
                }
            }
            if (resetB.clicked(&window)) {
                tokens = {};
                ArrowShooters = {};
                Checkpoints = {};
                loadlevel("data/levels/level" + to_string(level) + ".json");
                lost = false;
                lastPlayerState = nullptr;
                LastTokenState = {};
            }
        }
        FillerUI::SetClicking(false);
        window.display();
    }
    return 0;
}
std::vector<GhostData> reloadGhostData() {
    string json;
    string line;
    ifstream f(lastfilename);
    if (f.is_open())
    {
        while (getline(f, line))
        {
            json = json + line;
        }
        f.close();
    }
    std::vector<GhostData> ghostdata;
    Document ghostFile;
    ghostFile.Parse(json.c_str());
    ghostLevel = ghostFile["level"].GetInt();
    for (int i = 0; i < ghostFile["data"].Capacity(); i++) {
        const Value& data = ghostFile["data"][i];
        GhostData gd;
        gd.time = data["time"].GetInt();
        gd.pPos = { data["pX"].GetInt(), data["pY"].GetInt() };
        ghostdata.push_back(gd);
    }
    return ghostdata;
}
void saveGhostData(std::vector<GhostData> *gd) {
    std::string filename = lastfilename;
    std::string leveldata = R"("level": )";
    leveldata = leveldata + to_string(ghostLevel) + ",";
    std::string result = "{";
    std::string data = R"("data":[)";
    data = data + R"({"pX": )" + to_string(startingpos.x) + R"(, "pY": )" + to_string(startingpos.y) + R"(, "time": 0},)";
    for (int i = 0; i < gd->size(); i++) {
        std::string arraything = "{";
        arraything = arraything + R"("pX": )" + std::to_string((*gd)[i].pPos.x) + ",";
        arraything = arraything + R"("pY": )" + std::to_string((*gd)[i].pPos.y) + ",";
        arraything = arraything + R"("time": )" + std::to_string((*gd)[i].time) + "}";
        if (i < gd->size() - 1)
            arraything = arraything + ",";
        data = data + arraything;
    }
    data = data + "]";
    result = result + leveldata + data + "}";
    ofstream resultf;
    resultf.open(filename);
    std::cout << result;
    resultf << result;
    resultf.close();
}
std::vector<GhostData> loadGhostData(){
    char filename[MAX_PATH];
        OPENFILENAME ofn;
        ZeroMemory(&filename, sizeof(filename));
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = window.getSystemHandle();
        ofn.lpstrFilter = "Ghost Files\0*.ghost\0Any File\0*.*\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = "Open Ghost File";
        ofn.Flags = OFN_DONTADDTORECENT;

        if (!GetOpenFileNameA(&ofn)) {
            switch (CommDlgExtendedError())
            {
            case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n";   break;
            case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n";  break;
            case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n";  break;
            case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n";  break;
            case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n";  break;
            case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n";  break;
            case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
            case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
            case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n";     break;
            case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n";          break;
            case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n";      break;
            case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n";      break;
            case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
            case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n"; break;
            case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
            default: std::cout << "You cancelled.\n";
            }
        }
    string json;
    string line;
    ifstream f(filename);
    if (f.is_open())
    {
        while (getline(f, line))
        {
            json = json + line;
        }
        f.close();
    }
    lastfilename = filename;
    std::vector<GhostData> ghostdata;
    Document ghostFile;
    ghostFile.Parse(json.c_str());
    ghostLevel = ghostFile["level"].GetInt();
    for (int i = 0; i < ghostFile["data"].Capacity(); i++) {
        const Value& data = ghostFile["data"][i];
        GhostData gd;
        gd.time = data["time"].GetInt();
        gd.pPos = { data["pX"].GetInt(), data["pY"].GetInt() };
        ghostdata.push_back(gd);
    }
    return ghostdata;
}
int ghostMain()
{
    bool elegibleToMove = false;
    sf::View mainView({ 160.f, 160.f }, { 320.f, 320.f });
    sf::View ghostView({ 480.f, 160.f }, { 320.f, 320.f });
    mainView.setViewport(sf::FloatRect(0.f, 0.f, 0.5f, 1.f));
    ghostView.setViewport(sf::FloatRect(0.5f, 0.f, 0.5f, 1.f));
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);
    wstex.loadFromFile("data/winscreen.png");
    winscreen.setTexture(wstex);
    ltex.loadFromFile("data/losescreen.png");
    losescreen.setTexture(ltex);
    estex.loadFromFile("data/endscreen_ORISIT.png");
    endscreen.setTexture(estex);
    losescreen.setPosition(0, -48);
    winscreen.setPosition(0, -48);
    endscreen.setPosition(0, -48);
    FillerUI::TextButton nextLvlB;
    nextLvlB.text = "Set Ghost";
    nextLvlB.rect.setPosition(50, 200);
    FillerUI::TextButton exitB;
    exitB.text = "Exit";
    exitB.rect.setPosition(320 - 16 - exitB.sizeX, 200);
    FillerUI::TextButton retryB;
    retryB.text = "Retry";
    retryB.rect.setPosition(16, 200);
    FillerUI::TextButton resetB;
    resetB.text = "Reset";
    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir("./data/levels")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            string name = string(ent->d_name);
            if (name.find("level") != std::string::npos && name.find(".json") != std::string::npos) {
                std::string beforeJson = process(name, ".json");
                std::string afterLevel = beforeJson.substr(beforeJson.find("level") + 5);
                if (isNumber(afterLevel)) {
                    levels++;
                }
            }
        }
        closedir(dir);
    }
    std::cout << "before dialog\n";
    std::vector<GhostData> ghostdata = loadGhostData();
    std::vector<GhostData> savedata;
    std::cout << "data/levels/level" + to_string(ghostLevel) + ".json" << std::endl;
    std::cout << "after dialog\n";
    if (!SetCurrentDirectoryA(getexepath().c_str())) { printf("SetCurrentDirectory failed (%d)\n", GetLastError()); }
    loadlevel("data/levels/level" + to_string(ghostLevel) + ".json");
    window.setSize({ 640, 320 });
    gTokens = tokens;
    gArrowShooters = ArrowShooters;
    gCheckpoints = Checkpoints;
    for (int i = 0; i < gTokens.size(); i++) {
        gTokens[i].x += 320 / 16;
    }
    for (int i = 0; i < gArrowShooters.size(); i++) {
        gArrowShooters[i].x += 320 / 16;
    }
    for (int i = 0; i < gCheckpoints.size(); i++) {
        gCheckpoints[i].x += 320 / 16;
    }
    Timer timer;
    timer.start();
    while (window.isOpen())
    {
        sf::Event event;
        for (size_t i = 0; i < tokens.size(); i++) {
            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                tokens[i].timestouched = 1;
                tokens[i].tokencolor = sf::Color::Blue;
            }
        }
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonReleased)
                FillerUI::SetClicking(true);
            if (event.type == sf::Event::KeyPressed) {
                if (!lost && !won && !ghostWon) {
                    elegibleToMove = true;
                    if (event.key.code == sf::Keyboard::E && appInFocus(&window) && elegibleToMove) {
                        sf::Vector2 lastpos = { player.x, player.y };
                        player.y--;
                        player.x++;
                        bool touched = false;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.y = lastpos.y;
                            player.x = lastpos.x;
                        }
                        elegibleToMove = false;
                    }
                    if (event.key.code == sf::Keyboard::D && appInFocus(&window) && elegibleToMove) {
                        sf::Vector2 lastpos = { player.x, player.y };
                        player.y++;
                        player.x++;
                        bool touched = false;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.y = lastpos.y;
                            player.x = lastpos.x;
                        }
                        elegibleToMove = false;
                    }
                    if (event.key.code == sf::Keyboard::W && appInFocus(&window) && elegibleToMove) {
                        sf::Vector2 lastpos = { player.x, player.y };
                        player.y--;
                        player.x--;
                        bool touched = false;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.y = lastpos.y;
                            player.x = lastpos.x;
                        }
                        elegibleToMove = false;
                    }
                    if (event.key.code == sf::Keyboard::S && appInFocus(&window) && elegibleToMove) {
                        sf::Vector2 lastpos = { player.x, player.y };
                        player.y++;
                        player.x--;
                        bool touched = false;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.y = lastpos.y;
                            player.x = lastpos.x;
                        }
                        elegibleToMove = false;
                    }
                    if (event.key.code == sf::Keyboard::Key::Down && appInFocus(&window) && elegibleToMove) {
                        int lastpos = player.y;
                        bool touched = false;
                        player.y++;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.y = lastpos;
                        }
                        elegibleToMove = false;
                    }
                    if (event.key.code == sf::Keyboard::Key::Up && appInFocus(&window) && elegibleToMove) {
                        int lastpos = player.y;
                        bool touched = false;
                        player.y--;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.y = lastpos;
                        }
                        elegibleToMove = false;
                    }
                    if (event.key.code == sf::Keyboard::Key::Left && appInFocus(&window) && elegibleToMove) {
                        int lastpos = player.x;
                        bool touched = false;
                        player.x--;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.x = lastpos;
                        }
                        elegibleToMove = false;
                    }
                    if (event.key.code == sf::Keyboard::Key::Right && appInFocus(&window) && elegibleToMove) {
                        int lastpos = player.x;
                        bool touched = false;
                        player.x++;
                        for (size_t i = 0; i < tokens.size(); i++) {
                            if (tokens[i].x == player.x && tokens[i].y == player.y) {
                                touched = true;
                            }
                        }
                        for (size_t i = 0; i < Checkpoints.size(); i++) {
                            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                                touched = true;
                            }
                        }
                        if (!touched) {
                            player.x = lastpos;
                        }
                        elegibleToMove = false;
                    }
                }
                if ((event.key.code == sf::Keyboard::E || event.key.code == sf::Keyboard::S || event.key.code == sf::Keyboard::D || event.key.code == sf::Keyboard::W || event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Right)){// && appInFocus(&window)) {
                    std::cout << "event received" << std::endl;
                    GhostData newgd;
                    newgd.pPos.x = player.x;
                    newgd.pPos.y = player.y;
                    newgd.time = timer.elapsedMilliseconds();
                    savedata.push_back(newgd);
                }
            }
        }
        won = true;
        window.setView(mainView);
        window.clear();
        for (size_t i = 0; i < tokens.size(); i++) {
            Token token = tokens[i];
            if (token.x == player.x && token.y == player.y) {
                if (!token.debounce) {
                    token.timestouched = token.timestouched + 1;
                    token.tokencolor = sf::Color::Blue;
                }
                token.debounce = true;
            }
            else {
                token.debounce = false;
            }
            tokens[i] = token;
            if (token.timestouched > 1)
                lost = true;
            token.draw(&window);
        }
        for (size_t i = 0; i < tokens.size(); i++) {
            if (tokens[i].timestouched == 0)
                won = false;
        }
        for (size_t i = 0; i < Checkpoints.size(); i++) {
            Checkpoints[i].draw(&window);
        }
        for (int i = 0; i < ArrowShooters.size(); i++) {
            sf::Texture cTex;
            cTex.loadFromFile("data/hmmm.png");
            sf::Sprite cSpr;
            cSpr.setTexture(cTex);
            cSpr.setPosition(player.player.getPosition());
            if (Collision::PixelPerfectTest(ArrowShooters[i].arrow.Sarrow, cSpr))
                lost = true;
            ArrowShooters[i].draw(&window);
        }
        for (size_t i = 0; i < Checkpoints.size(); i++) {
            if (Checkpoints[i].x == player.x && Checkpoints[i].y == player.y) {
                Checkpoints[i].timestouched++;
                if (Checkpoints[i].timestouched <= 1) {
                    for (int i1 = 0; i1 < Checkpoints.size(); i1++) {
                        if (i1 != i) {
                            Checkpoints[i1].timestouched = 0;
                        }
                    }
                    LastTokenState = tokens;
                    LastTokenStateLevel = level;
                    lastPlayerState = new Player(player.x, player.y);
                }
            }
        }
        player.draw(&window);
        if (won) {
            bool nextlevel = level + 1 <= levels;
            exitB.rect.setPosition(320 - 50 - exitB.sizeX, 200);
            for (int i = 0; i < ArrowShooters.size(); i++) {
                ArrowShooters[i].stop = true;
            }
            if (nextlevel) {
                nextLvlB.draw(&window);
                window.draw(winscreen);
            }
            else {
                window.draw(endscreen);
                float Width1 = 320;
                float Width2 = exitB.rect.getGlobalBounds().width;
                float X2 = exitB.rect.getPosition().x;
                exitB.rect.setPosition(((Width1 - Width2) / 2), 200);
            }

            exitB.draw(&window);
            if (exitB.clicked(&window))
                return 0;
            if (nextLvlB.clicked(&window) && nextlevel) {
                saveGhostData(&savedata);
            }
        }
        if (lost || ghostWon) {
            exitB.rect.setPosition(320 - 16 - exitB.sizeX, 200);
            window.draw(losescreen);
            for (int i = 0; i < ArrowShooters.size(); i++) {
                ArrowShooters[i].stop = true;
            }
            exitB.draw(&window);
            retryB.draw(&window);
            float Width1 = 320;
            float Width2 = resetB.rect.getGlobalBounds().width;
            float X2 = resetB.rect.getPosition().x;
            resetB.rect.setPosition(((Width1 - Width2) / 2), 200);
            resetB.draw(&window);
            if (exitB.clicked(&window))
                return 0;
            if (retryB.clicked(&window)) {
                bool touched = false;
                for (int i = 0; i < Checkpoints.size(); i++) {
                    if (Checkpoints[i].timestouched > 0) {
                        touched = true;
                    }
                }
                if (touched) {
                    tokens = LastTokenState;
                    player = *lastPlayerState;
                    lost = false;
                    for (int i = 0; i < ArrowShooters.size(); i++) {
                        ArrowShooters[i].stop = false;
                    }
                    ArrowShooters = {};
                    string json;
                    string line;
                    ifstream f("data/levels/level" + to_string(level) + ".json");
                    if (f.is_open())
                    {
                        while (getline(f, line))
                        {
                            json = json + line;
                        }
                        f.close();
                    }
                    Document level1;
                    level1.Parse(json.c_str());
                    if (level1.HasMember("ArrowShooters")) {
                        for (size_t i = 0; i < level1["ArrowShooters"].Size(); i++) {
                            const Value& Arrowshooter1 = level1["ArrowShooters"][i];
                            ArrowShooter Arrowshooter;
                            Arrowshooter.x = Arrowshooter1["x"].GetInt();
                            Arrowshooter.y = Arrowshooter1["y"].GetInt();
                            Arrowshooter.type = Arrowshooter1["type"].GetInt();
                            ArrowShooters.insert({ i, Arrowshooter });
                        }
                    }
                }
                if (!touched) {
                    tokens = {};
                    ArrowShooters = {};
                    Checkpoints = {};
                    savedata.resize(0);
                    savedata = {};
                    loadlevel("data/levels/level" + to_string(level) + ".json");
                    for (int i = 0; i < gTokens.size(); i++) {
                        gTokens[i].tokencolor = sf::Color::Yellow;
                        gTokens[i].timestouched = 0;
                    }
                    ghostdata = reloadGhostData();
                    timer.stop();
                    timer.start();
                    lost = false;
                }
            }
            if (resetB.clicked(&window)) {
                tokens = {};
                ArrowShooters = {};
                Checkpoints = {};
                loadlevel("data/levels/level" + to_string(level) + ".json");
                for (int i = 0; i < gTokens.size(); i++) {
                    gTokens[i].tokencolor = sf::Color::Yellow;
                    gTokens[i].timestouched = 0;
                }
                ghostdata = reloadGhostData();
                timer.stop();
                timer.start();
                lost = false;
                lastPlayerState = nullptr;
                LastTokenState = {};
            }
        }
        window.setView(ghostView);
        Player ghostPlayer(0, 0);
        bool hit = false;
        ghostWon = true;
        for (int i = 0; i < ghostdata.size(); i++) {
            //std::cout << i << ": pX: " << ghostdata[i].pPos.x << ", time: " << ghostdata[i].time << std::endl;
            if (timer.elapsedMilliseconds() >= ghostdata[i].time && timer.elapsedMilliseconds() <= ghostdata[i + (ghostdata.size() > i)].time && !won && !lost) {
                hit = true;
                ghostPlayer = Player(ghostdata[i].pPos.x + 320/16, ghostdata[i].pPos.y);
            }
            if ((ghostdata.size() == i) && timer.elapsedMilliseconds() >= ghostdata[i].time) {
                std::cout << "hello there" << std::endl;
                hit = true;
                ghostPlayer = Player(ghostdata[i].pPos.x + 320 / 16, ghostdata[i].pPos.y);
            }
        }
        if (timer.elapsedMilliseconds() < ghostdata[1].time) {
            ghostPlayer = Player(ghostdata[0].pPos.x + 320/16, ghostdata[0].pPos.y);
        }
        for (size_t i = 0; i < gTokens.size(); i++) {
            Token token = gTokens[i];
            if (gTokens[i].x == ghostPlayer.x && token.y == ghostPlayer.y) {
                if (!token.debounce) {
                    token.timestouched = token.timestouched + 1;
                    token.tokencolor = sf::Color::Blue;
                }
                token.debounce = true;
            }
            else {
                token.debounce = false;
            }
            gTokens[i] = token;
            token.draw(&window);
        }
        for (size_t i = 0; i < gTokens.size(); i++) {
            if (gTokens[i].timestouched == 0)
                ghostWon = false;
            if (gTokens[i].x - 320 / 16 == ghostdata[0].pPos.x && gTokens[i].y == ghostdata[0].pPos.y) {
                if (gTokens[i].timestouched < 1) {
                    gTokens[i].tokencolor = sf::Color::Blue;
                    gTokens[i].timestouched = 1;
                }
            }
        }
        for (size_t i = 0; i < gCheckpoints.size(); i++) {
            gCheckpoints[i].draw(&window);
        }
        for (int i = 0; i < gArrowShooters.size(); i++) {
            gArrowShooters[i].draw(&window);
        }
        ghostPlayer.draw(&window);
        //std::cout << timer.elapsedMilliseconds() << std::endl;
        FillerUI::SetClicking(false);
        window.display();
    }
    return 0;
}
int mainMenu();
int ghostMenu();
void eraseSubStr(std::string & mainStr, const std::string & toErase)
{
    size_t pos = mainStr.find(toErase);
    if (pos != std::string::npos)
    {
        mainStr.erase(pos, toErase.length());
    }
}
int createGhost() {
    if (!SetCurrentDirectoryA((getexepath() + "\\data\\ghostData").c_str())) { printf("SetCurrentDirectory failed (%d)\n", GetLastError()); }
    char filename[MAX_PATH];
    OPENFILENAME ofn;
    ZeroMemory(&filename, sizeof(filename));
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window.getSystemHandle();
    ofn.lpstrFilter = "Ghost Files\0*.ghost\0Any File\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Open Ghost File";
    ofn.Flags = OFN_DONTADDTORECENT;

    if (!GetOpenFileNameA(&ofn)) {
        switch (CommDlgExtendedError())
        {
        case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n";   break;
        case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n";  break;
        case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n";  break;
        case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n";  break;
        case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n";  break;
        case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n";  break;
        case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
        case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
        case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n";     break;
        case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n";          break;
        case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n";      break;
        case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n";      break;
        case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
        case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n"; break;
        case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
        default: std::cout << "You cancelled.\n";
        }
    }
    if (!SetCurrentDirectoryA((getexepath() + "\\data\\levels").c_str())) { printf("SetCurrentDirectory failed (%d)\n", GetLastError()); }
    char filename1[MAX_PATH];
    OPENFILENAME ofn1;
    ZeroMemory(&filename1, sizeof(filename1));
    ZeroMemory(&ofn1, sizeof(ofn1));
    ofn1.lStructSize = sizeof(ofn1);
    ofn1.hwndOwner = window.getSystemHandle();
    ofn1.lpstrFilter = "Level Files\0*.json\0Any File\0*.*\0";
    ofn1.lpstrFile = filename1;
    ofn1.nMaxFile = MAX_PATH;
    ofn1.lpstrTitle = "Open Level File";
    ofn1.Flags = OFN_DONTADDTORECENT;

    if (!GetOpenFileNameA(&ofn1)) {
        switch (CommDlgExtendedError())
        {
        case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n";   break;
        case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n";  break;
        case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n";  break;
        case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n";  break;
        case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n";  break;
        case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n";  break;
        case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
        case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
        case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n";     break;
        case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n";          break;
        case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n";      break;
        case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n";      break;
        case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
        case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n"; break;
        case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
        default: std::cout << "You cancelled.\n";
        }
    }
    std::cout << filename1 << std::endl;
    std::string level = filename1;
    std::cout << level << std::endl;
    std::string afterLevel = level.substr(level.find_last_of("level") + 1);
    eraseSubStr(afterLevel, ".json");
    ofstream fs;
    fs.open(filename);
    fs << R"({"level": )" << afterLevel << R"(, "data":[{"pX": 0, "pY": 0, "time": 0}, {"pX": 0, "pY": 0, "time": 0}]})";
    fs.close();
    std::cout << R"({"level": )" << afterLevel << R"(, "data":[{"pX": 0, "pY": 0, "time": 0}, {"pX": 0, "pY": 0, "time": 0}]})" << std::endl;
    std::cout << "created an empty ghost on level " << afterLevel << std::endl;
    if (!SetCurrentDirectoryA(getexepath().c_str())) { printf("SetCurrentDirectory failed (%d)\n", GetLastError()); }
    ghostMenu();
}
int ghostMenu() {
    FillerUI::SetClicking(false);
    sf::SoundBuffer hoverBuffer;
    hoverBuffer.loadFromMemory(buttonHover, buttonHover_length);
    sf::Sound hoverSound;
    hoverSound.setBuffer(hoverBuffer);
    bool playDebounce = false;
    bool createDebounce = false;
    bool backDebounce = false;
    FillerUI::HoverSensitiveTextButton playB;
    playB.sizeX += 20;
    playB.text = "Play";
    playB.rect.setPosition((320 / 2) - (playB.sizeX / 2), 155);
    FillerUI::HoverSensitiveTextButton createB;
    createB.sizeX += 20;
    createB.text = "Create Ghost";
    createB.rect.setPosition((320 / 2) - (createB.sizeX / 2), 200);
    FillerUI::HoverSensitiveTextButton backB;
    backB.sizeX += 20;
    backB.text = "Back to Menu";
    backB.rect.setPosition((320 / 2) - (backB.sizeX / 2), 245);
    sf::Text sfText;
    sf::Font sfFont;
    sfFont.loadFromMemory(ArcadeTTF, ArcadeTTF_length);
    sfText.setFont(sfFont);
    sfText.setString("Filler");
    sfText.setCharacterSize(32);
    sfText.setFillColor(sf::Color::Blue);
    float textWidth = sfText.getGlobalBounds().width;
    float textHeight = sfText.getGlobalBounds().height;
    sfText.setPosition(320 / 2 - textWidth / 2, 100);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                    FillerUI::SetClicking(true);
                if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
                    FillerUI::SetRightClicking(true);
            }
        }
        window.clear();
        if (playB.hovering(&window)) {
            if (!playDebounce) {
                hoverSound.play();
                playDebounce = true;
            }
        }
        else {
            playDebounce = false;
        }
        if (createB.hovering(&window)) {
            if (!createDebounce) {
                hoverSound.play();
                createDebounce = true;
            }
        }
        else {
            createDebounce = false;
        }
        if (backB.hovering(&window)) {
            if (!backDebounce) {
                hoverSound.play();
                backDebounce = true;
            }
        }
        else {
            backDebounce = false;
        }
        if (playB.clicked(&window)) {
            ghostMain();
            break;
        }
        if (createB.clicked(&window)) {
            createGhost();
            break;
        }
        if (backB.clicked(&window)) {
            mainMenu();
            break;
        }
        playB.draw(&window);
        createB.draw(&window);
        backB.draw(&window);
        window.draw(sfText);
        FillerUI::SetClicking(false);
        window.display();
    }
    return 0;
}
int mainMenu() {
    sf::SoundBuffer hoverBuffer;
    hoverBuffer.loadFromMemory(buttonHover, buttonHover_length);
    sf::Sound hoverSound;
    hoverSound.setBuffer(hoverBuffer);
    bool spDebounce = false;
    bool ghostDebounce = false;
    FillerUI::HoverSensitiveTextButton spButton;
    spButton.sizeX += 20;
    spButton.text = "Singleplayer";
    spButton.rect.setPosition((320 / 2) - (spButton.sizeX / 2), 155);
    FillerUI::HoverSensitiveTextButton ghostB;
    ghostB.sizeX += 20;
    ghostB.text = "Ghost Mode";
    ghostB.rect.setPosition((320 / 2) - (ghostB.sizeX / 2), 200);
    sf::Text sfText;
    sf::Font sfFont;
    sfFont.loadFromMemory(ArcadeTTF, ArcadeTTF_length);
    sfText.setFont(sfFont);
    sfText.setString("Filler");
    sfText.setCharacterSize(32);
    sfText.setFillColor(sf::Color::Blue);
    float textWidth = sfText.getGlobalBounds().width;
    float textHeight = sfText.getGlobalBounds().height;
    sfText.setPosition(320 / 2 - textWidth / 2, 100);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                    FillerUI::SetClicking(true);
                if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
                    FillerUI::SetRightClicking(true);
            }
        }
        window.clear();
        if (spButton.hovering(&window)) {
            if (!spDebounce) {
                hoverSound.play();
                spDebounce = true;
            }
        }
        else {
            spDebounce = false;
        }
        if (ghostB.hovering(&window)) {
            if (!ghostDebounce) {
                hoverSound.play();
                ghostDebounce = true;
            }
        }
        else {
            ghostDebounce = false;
        }
        if (spButton.clicked(&window)) {
            singlePlayerMain();
            break;
        }
        if (ghostB.clicked(&window)) {
            ghostMenu();
            break;
        }
        spButton.draw(&window);
        ghostB.draw(&window);
        window.draw(sfText);
        FillerUI::SetClicking(false);
        window.display();
    }
    return 0;
}
int main() {
    mainMenu();
    return 0;
}