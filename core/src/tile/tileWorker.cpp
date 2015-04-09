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
    m_reqCount = 0;
}

void TileWorker::abort() {
    m_aborted = true;
}

void TileWorker::update() {

    if (m_finished) {
        return;
    }

    static int reqFetched = 0;

    for (auto& req : m_requests) {
        if (req->m_handled) {
            reqFetched++;

            MapTile* tile = req->m_tile;
            View* view = (View*) req->m_view;

            logMsg("Request data size %d, for tile %d %d %d\n", req->m_size, tile->getID().x, tile->getID().y, tile->getID().z);

            std::stringstream out;
            out << req->m_rawData;

            logMsg("%s\n", req->m_rawData);

            std::shared_ptr<TileData> tileData = req->m_dataSource->parse(*tile, out);

            std::vector<std::unique_ptr<Style>>* styles = (std::vector<std::unique_ptr<Style>> *) req->m_styles;

            if (tileData) {
                logMsg("adding data\n");
            //    for (auto& style : *styles) {
            //        style->addData(*tileData, *tile, view->getMapProjection());
            //        tile->update(0, *style, *view);
            //    }
            } else {
                logMsg("no tile data");
            }

            free(req->m_rawData);
            delete req;
        }
    }

    if (reqFetched == m_reqCount) {
        reqFetched = 0;
        m_reqCount = 0;
        m_finished = true;
        m_requests.clear();
    }
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
        auto req = new DataSource::DataReq {
            (DataSource *) dataSource.get(), nullptr, 0, m_tile.get(), (void*) &_styles, (void*) &_view, false
        };
        m_requests.push_back(req);
        m_reqCount++;
        if (! dataSource->loadTileData(*m_tile, req)) {
            logMsg("ERROR: Loading failed for tile [%d, %d, %d]\n", _tile.z, _tile.x, _tile.y);
            continue;
        }
    }

}

std::shared_ptr<MapTile> TileWorker::getTileResult() {

    m_free = true;
    return m_tile;

}

