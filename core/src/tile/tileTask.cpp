#include "tileTask.h"
#include "tile.h"
#include "tileManager.h"
#include "data/dataSource.h"

namespace Tangram {

TileTask::TileTask(TileManager& _tileManager, std::shared_ptr<Tile> _tile) :
    tile(_tile),
    tileManager(_tileManager){
    for (auto& source : _tileManager.dataSources()) {
        items.push_back({ source.get(), nullptr, State::none });
    }
}

void TileTask::cancel() {
    for (auto& item : items) {
        if (item.state == State::loading) {
            item.source->cancelLoadingTile(tile->getID());
            tileManager.removeLoadPending();
        }
        item.state = State::canceled;
    }
}

bool TileTask::awaitsLoading() {
    for (auto& item : items) {
        if (item.state == State::none) { return true; }
    }
    return false;

}


void TileTask::load() {
    for (auto& item : items) {
        if (item.state != State::none) { continue; }

        TileID id = tile->getID();
        logMsg("Load tile [%d, %d, %d]\n", id.z, id.x, id.y);

        if (item.source->getTileData(*this)) {
            logMsg("USE RAW CACHE\n");
            continue;
        }

        if (tileManager.addLoadPending()) {
            item.state = State::loading;
            logMsg("Load tile [%d, %d, %d] ==> set state\n", id.z, id.x, id.y);

            if (!item.source->loadTileData(shared_from_this())) {

                logMsg("ERROR: Loading failed for tile [%d, %d, %d]\n",
                       id.z, id.x, id.y);

                //item.state = State::not_available;
                tileManager.removeLoadPending();
            }
        }
    }
}

void TileTask::process(DataSource* source, std::shared_ptr<std::vector<char>> _rawData) {
    bool ready = true;

    TileID id = tile->getID();
    logMsg("Processed tile [%d, %d, %d]\n", id.z, id.x, id.y);

    for (auto& item : items) {
        if (item.source == source) {
            if (item.state == State::loading) {
                tileManager.removeLoadPending();
            }
            item.rawData = _rawData;
            item.state = State::processing;
        } else {
            if (item.state != State::processing)
                ready = false;
        }
    }
    if (ready) {
        logMsg("Ready tile [%d, %d, %d]\n", id.z, id.x, id.y);
        tileManager.tileLoaded(shared_from_this());
    }
}


}
