#include "room_loader.h"
#include "resource.h"
#include "tile.h"
#include "layer.h"
#include "camera.h"
#include "room.h"
#include "entity.h"
#include "entity_factory.h"
#include "vec2.h"
#include <string>
#include <sstream>
#include <vector>

Room* loadRoom(const std::string& levelFile) {
    std::string roomData = Resource::getText(levelFile);

    // Split into lines
    std::vector<std::string> data;
    {
        std::istringstream stream(roomData);
        std::string line;
        while (std::getline(stream, line)) {
            // Trim whitespace
            size_t start = line.find_first_not_of(" \t\r\n");
            size_t end = line.find_last_not_of(" \t\r\n");
            if (start != std::string::npos)
                data.push_back(line.substr(start, end - start + 1));
            else
                data.push_back("");
        }
    }

    int idx = 0;
    auto next = [&]() -> std::string {
        if (idx < static_cast<int>(data.size()))
            return data[idx++];
        return "";
    };
    auto nextInt = [&]() -> int {
        return std::stoi(next());
    };

    // Tile definitions
    int numTileTypes = nextInt();
    std::vector<Tile> tiles;

    for (int i = 0; i < numTileTypes; i++) {
        std::string filename = next();
        int x = nextInt();
        int y = nextInt();
        int w = nextInt();
        int h = nextInt();
        int collidable = nextInt();
        int hookable = nextInt();
        tiles.push_back(Tile(Resource::getBitmap(filename), x, y, w, h, hookable == 1, collidable == 1));
    }

    // Background layer
    int width = nextInt();
    int height = nextInt();
    Layer* bgLayer = new Layer(width, height);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            int t = nextInt();
            if (t != -1) bgLayer->setTile(x, y, tiles[t]);
        }
    }

    // Middle layer
    width = nextInt();
    height = nextInt();
    Layer* midLayer = new Layer(width, height);
    Tile emptyTile;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            int t = nextInt();
            midLayer->setTile(x, y, t != -1 ? tiles[t] : emptyTile);
        }
    }

    int midWidth = width;
    int midHeight = height;

    // Foreground layer
    width = nextInt();
    height = nextInt();
    Layer* fgLayer = new Layer(width, height);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            int t = nextInt();
            if (t != -1) fgLayer->setTile(x, y, tiles[t]);
        }
    }

    Room* room = new Room(bgLayer, midLayer, fgLayer,
        new Camera(Vec2(0, 0), Vec2(static_cast<float>(midWidth * 10), static_cast<float>(midHeight * 10))));

    // Entities
    width = nextInt();
    height = nextInt();
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            int entityId = nextInt();
            if (entityId != -1) {
                Entity* entity = EntityFactory::create(entityId);
                if (!entity) continue;
                Vec2 pos(10.0f * x + 5.0f, 10.0f * (y + 1) - entity->getHalfSize().y);
                entity->setPosition(pos);
                room->addEntity(entity);
            }
        }
    }

    // Camera rects
    int rectCount = nextInt();
    for (int i = 0; i < rectCount; i++) {
        float rx = static_cast<float>(nextInt());
        float ry = static_cast<float>(nextInt());
        float rw = static_cast<float>(nextInt());
        float rh = static_cast<float>(nextInt());
        room->getCamera()->addRect(rx, ry, rw, rh);
    }

    return room;
}
