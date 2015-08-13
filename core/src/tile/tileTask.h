#pragma once

#include <vector>
#include <memory>
#include <map>

namespace Tangram {

class DataSource;
class Tile;
class TileManager;

class TileTask : public std::enable_shared_from_this<TileTask> {

public:
    enum class State { none, loading, processing, ready, canceled };

    struct Item {
        DataSource* source;
        std::shared_ptr<std::vector<char>> rawData;
        State state;
    };

    std::shared_ptr<Tile> tile;
    TileManager &tileManager;

    std::vector<Item> items;

    TileTask(TileManager& _tileManager, std::shared_ptr<Tile> _tile);

    TileTask& operator=(const TileTask& _other) = delete;

    void cancel();

    void load();

    void process(DataSource* source, std::shared_ptr<std::vector<char>> _rawData);

    bool awaitsLoading();

};

typedef std::function<void(std::shared_ptr<TileTask>&&)> TileTaskCb;

}
