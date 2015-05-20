#include "labelContainer.h"
#include "tile/mapTile.h"

LabelContainer::LabelContainer() {}

LabelContainer::~LabelContainer() {
    m_labelUnits.clear();
    m_pendingLabelUnits.clear();
}

bool LabelContainer::addLabel(MapTile& _tile, const std::string& _styleName, LabelTransform _transform, std::string _text, Label::Type _type) {
    
    auto currentBuffer = m_ftContext->getCurrentBuffer();

    if (currentBuffer) {
        fsuint textID = currentBuffer->genTextID();
        std::shared_ptr<Label> l(new Label(_transform, _text, textID, _type));
        
        l->rasterize(currentBuffer);
        l->update(m_viewProjection * _tile.getModelMatrix(), m_screenSize, 0);
        std::unique_ptr<TileID> tileID(new TileID(_tile.getID()));
        _tile.addLabel(_styleName, l);
        
        // lock concurrent collection
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_pendingLabelUnits.emplace_back(LabelUnit(l, tileID, _styleName));
        }

        return true;
    }

    return false;
}

void LabelContainer::updateOcclusions() {
    
    /*
     * update placed labels
     */
    for(int i = 0; i < m_placedLabels.size(); i++) {
        auto label1 = m_placedLabels[i].lock();
        if(label1 == nullptr) {
            m_placedLabels[i--] = std::move(m_placedLabels[m_placedLabels.size() - 1]);
            m_placedLabels.pop_back();
            continue;
        }
        for(int j = i+1; j < m_placedLabels.size(); j++) {
            auto label2 = m_placedLabels[j].lock();
            if(label2 == nullptr) {
                m_placedLabels[j--] = std::move(m_placedLabels[m_placedLabels.size() - 1]);
                m_placedLabels.pop_back();
                continue;
            }
            label1->setVisible(true);
            label2->setVisible(true);
            if(label1->getAABB().intersect(label2->getAABB())) {
                if(intersect(label1->getOBB(), label2->getOBB())) {
                    label2->setVisible(false);
                    m_placedLabels[j--] = std::move(m_placedLabels[m_placedLabels.size() - 1]);
                    m_placedLabels.pop_back();
                }
            }
        }
    }

    // merge pending labels from threads
    m_labelUnits.reserve(m_labelUnits.size() + m_pendingLabelUnits.size());
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_labelUnits.insert(m_labelUnits.end(), std::make_move_iterator(m_pendingLabelUnits.begin()), std::make_move_iterator(m_pendingLabelUnits.end()));
        std::vector<LabelUnit>().swap(m_pendingLabelUnits);
    }

    /*
     *  check all labels with placed labels
     */
    m_placedLabels.reserve(m_placedLabels.size() + m_labelUnits.size());
    for(int i = 0; i < m_labelUnits.size(); i++) {
        auto label = m_labelUnits[i].getWeakLabel();
        if(label == nullptr) {
            m_labelUnits[i--] = std::move(m_labelUnits[m_labelUnits.size() - 1]);
            m_labelUnits.pop_back();
            continue;
        }
        if ((label->isCollisionChecked() && !label->isVisible()) || label->isOutOfScreen() || label->getType() == Label::Type::DEBUG) {
            continue;
        }
        label->setCollisionChecked(true);
        for(int j = 0; j < m_placedLabels.size(); j++) {
            auto placedLabel = m_placedLabels[j].lock();
            if(placedLabel == label) {
                continue;
            }
            if(placedLabel->getAABB().intersect(label->getAABB())) {
                if(intersect(placedLabel->getOBB(), label->getOBB())) {
                    /*
                     * already placed labels have more priority and hence make to other label is not visible
                     */
                    label->setVisible(false);
                    break;
                }
            }
            if(!label->isVisible()) {
                label->setVisible(true);
                m_placedLabels.push_back(m_labelUnits[i].getLabel());
            }
        }
        if(m_placedLabels.size() == 0) {
            label->setVisible(true);
            m_placedLabels.push_back(m_labelUnits[i].getLabel());
        }
    }
}
