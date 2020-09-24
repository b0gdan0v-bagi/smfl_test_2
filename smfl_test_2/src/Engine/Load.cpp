#include "engine.h"

using namespace sf;

bool Engine::loadImages()
{
    std::vector<std::string> imageName = { "bullet","player","easyEnemy", "skelletonEnemy" };
    for (std::vector<std::string>::iterator IMAGE = imageName.begin(); IMAGE != imageName.end(); ++IMAGE)
    {
        if (!imageList[*IMAGE].loadFromFile("resourses/images/" + *IMAGE + ".png"))
        {
            std::cout << "Cannot load " + *IMAGE + " image!";
            return false;
        }
    }

    imageList["skelletonEnemy"].createMaskFromColor(Color(255, 255, 255));
    imageList["easyEnemy"].createMaskFromColor(Color(0, 0, 0));
    imageList["player"].createMaskFromColor(Color(255, 255, 255));
    return true;
}
bool Engine::loadAnimations()
{
    animationManagerList["player"].create("walk", imageList["player"], 0, 0, 40, 80, 3, 0.01, 40);
    animationManagerList["player"].create("stay", imageList["player"], 0, 0, 40, 80, 1, 0);
    animationManagerList["player"].create("jump", imageList["player"], 0, 241, 40, 80, 1, 0);
    //animationManagerList["player"].create("duck", imageList["player"], 0, 161, 40, 80, 1, 0);
    animationManagerList["player"].create("duck", imageList["player"], 0, 0, 40, 40, 1, 0);
    animationManagerList["player"].create("die", imageList["player"], 0, 81, 40, 80, 3, 0.01, 40);
    animationManagerList["player"].setLoop("die");
    animationManagerList["easyEnemy"].create("move", imageList["easyEnemy"], 0, 0, 32, 32, 1, 0.005);
    animationManagerList["skelleton"].loadFromXML("resourses/images/skelleton.xml", imageList["skelletonEnemy"]);
    animationManagerList["bullet"].create("move", imageList["bullet"], 7, 10, 8, 8, 1, 0);
    animationManagerList["bullet"].create("explode", imageList["bullet"], 27, 7, 18, 18, 4, 0.01, 29, false);
    return true;
}

void Engine::gameRunning()

{
    window.setView(view);
    if (!levelChanger)
    {
        if (!menu.mainMenu(window, numberLevel)) return;
    }
    
    if (numberLevel / 100 != 0) pvp = true;
    else pvp = false;
    viewChanges(); // take view ports if screen splited or not
    entities.clear(); // delete memory for global vectors
    players.clear();
    playerBars.clear();
    lvl.clear(); 
    inGameKeyInputs = true; //make sure that keybord for players is working
    returnToMainMenu = false; //break bool for main cycle
    if (levelChanger) levelChanger = false; //level changer works once
    if (startGame()) // main cycle of game 
    {
        gameRunning(); //loop game runs
    }
}

void Engine::loadLevel()
{
    lvl.push_back(new TileMap);
    std::ostringstream numberLevelStream;
    numberLevelStream << numberLevel;
    // lvls for pvp have id > 100
    lvl[0]->load("resourses/maps/map" + numberLevelStream.str() + ".tmx");
    //Object player = lvl[0]->getObject("player1");
    //Object player2 = lvl[0]->getObject("player2");


    std::vector<Object> easyEnemy = lvl[0]->getObjectsByName("easyEnemy");
    std::vector<Object> skelleton = lvl[0]->getObjectsByName("skelleton");

    for (int i = 0; i < easyEnemy.size(); i++)
    {
        entities.push_back(new Enemy(animationManagerList["easyEnemy"],
            "EasyEnemy", *lvl[0], easyEnemy[i].rect.left, easyEnemy[i].rect.top));
    }
    for (int i = 0; i < skelleton.size(); i++)
    {
        entities.push_back(new Enemy(animationManagerList["skelleton"],
            "Skelleton", *lvl[0], skelleton[i].rect.left, skelleton[i].rect.top));
    }
    int numberOfPlayersToAdd;
    if (pvp) numberOfPlayersToAdd = 2;
    else numberOfPlayersToAdd = data.playersPVE;
    for (int i = 1; i <= numberOfPlayersToAdd; ++i)
    {
        std::ostringstream playerN;
        playerN << i;
        Object player = lvl[0]->getObject("player" + playerN.str());
        players.push_back(new Player(animationManagerList["player"], "Player" + playerN.str(), *lvl[0], player.rect.left, player.rect.top));
        std::cout << "player" + playerN.str() << " added!\n";
        playerBars.push_back(new statBar(font, pvp, i));
        //data.showFps = false; // we want fps counter only for 1st
    }
    std::cout << "players size = " << players.size() << "\n";
    std::cout << "players BAR size = " << playerBars.size() << "\n";
    //players.push_back(new Player(animationManagerList["player"], "Player1", *lvl[0], player.rect.left, player.rect.top));
    //players.push_back(new Player(animationManagerList["player"], "Player2", *lvl[0], player2.rect.left, player2.rect.top));

    //playerBars.push_back(new statBar(font, pvp, 1, data.showFps));
    //playerBars.push_back(new statBar(font, pvp, 2));

    std::cout << "\n=========================\n";
    std::cout << "Level number : " << numberLevel << " is succsessfully loaded\n" << "pvp set : " << pvp << "\n";
    std::cout << "=========================\n";
}

void Engine::readConfig()
{
    std::ifstream config;
    std::string line, var1, var2;
    bool correctResolution = false; //check standart resolutions! dont want to have parser
    std::vector<String> AvaliableResolutions = { "1280x720","1920x1080","3440x1440","1280x1024" };
    config.open("config.cfg");
    if (!config.is_open())
    {
        std::cout << "Cannot open config.cfg\nUsing standart variables!\n";
        resolution.x = 1280;
        resolution.y = 720;
        data.playersPVE = 1;
        data.showFps = true;
        std::ofstream configWrite("config.cfg");
        if (configWrite.is_open())
        {
            configWrite << "//Availiable resolutions = {1280x720,1920x1080,3440x1440,1280x1024}\n";
            configWrite << "resolution " << resolution.x << "x" << resolution.y << "\n";
            configWrite << "showFps " << data.showFps << "\n";
            configWrite << "PlayersPVE " << data.playersPVE << "\n";
            configWrite.close();
            std::cout << "Standart config created!\n";
        }
        else std::cout << "Unable to create config.cfg file\n";
        return;
    }
    while (std::getline(config, line))
    {
        std::istringstream iss(line);
        if (!(iss >> var1 >> var2)) { break; } // end
        if ((var1 == "resolution"))
        {
            correctResolution = false;
            if (var2 == "1280x720") {
                resolution.x = 1280; resolution.y = 720;  correctResolution = true;
            }
            if (var2 == "1920x1080") {
                resolution.x = 1920; resolution.y = 1080;  correctResolution = true;
            }
            if (var2 == "3440x1440") {
                resolution.x = 3440; resolution.y = 1440;  correctResolution = true;
            }
            if (var2 == "1280x1024") {
                resolution.x = 1280; resolution.y = 1024;  correctResolution = true;
            }
            if (!correctResolution) {
                resolution.x = 1280;
                resolution.y = 720;
                std::cout << "Cannot find correct resolution\n";
            }
            std::cout << "Resolution set to " << resolution.x << "x" << resolution.y << "\n";
        }
        if ((var1 == "showFps"))
        {
            if ((var2 == "1") || (var2 == "true")) {
                data.showFps = true;
            }
            else data.showFps = false;
            std::cout << "showFps set to " << data.showFps << "\n";
        }
        if ((var1 == "PlayersPVE"))
        {
            if (var2 == "1") {
                data.playersPVE = 1;
            }
            if (var2 == "2") {
                data.playersPVE = 2;
            }
            if (data.playersPVE > data.maxPlayersPVE) data.playersPVE = data.maxPlayersPVE;
            std::cout << "Number of PVE players set to " << data.playersPVE << "\n";
        }
    }
    std::cout << "Config readed!\n";
    config.close();
}

