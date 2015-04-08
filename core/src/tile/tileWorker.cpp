#include "tileWorker.h"
#include "platform.h"
#include "view/view.h"
#include "style/style.h"

#include <chrono>

TileWorker::TileWorker() {
    m_tileID.reset(new TileID(NOT_A_TILE));
    m_free = true;
    m_aborted = false;
    m_finished = false;
}

void TileWorker::abort() {
    m_aborted = true;
}

void TileWorker::load(const TileID &_tile,
                      const std::vector<std::unique_ptr<DataSource>> &_dataSources,
                      const std::vector<std::unique_ptr<Style>> &_styles,
                      const View& _view) {

    m_tileID.reset(new TileID(_tile));
    m_free = false;
    m_finished = false;
    m_aborted = false;
    m_tile = std::shared_ptr<MapTile>(new MapTile(_tile, _view.getMapProjection()));

    // Fetch tile data from data sources
    logMsg("Loading Tile [%d, %d, %d]\n", _tile.z, _tile.x, _tile.y);
    for (const auto& dataSource : _dataSources) {
        if (m_aborted) {
            m_finished = true;
        }
        if (! dataSource->loadTileData(*m_tile)) {
            logMsg("ERROR: Loading failed for tile [%d, %d, %d]\n", _tile.z, _tile.x, _tile.y);
            continue;
        }

        auto tileData = dataSource->getTileData(_tile);

        // Process data for all styles
        for (const auto& style : _styles) {
            if (m_aborted) {
                m_finished = true;
            }
            if (tileData) {
                style->addData(*tileData, *m_tile, _view.getMapProjection());
            }
            m_tile->update(0, *style, _view);
        }
    }

    m_finished = true;

}

std::shared_ptr<MapTile> TileWorker::getTileResult() {

    m_free = true;
    return m_tile;

}

