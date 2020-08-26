//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "midiportdatasender.h"

using namespace mu::midi;

void MidiPortDataSender::setMidiStream(std::shared_ptr<MidiStream> stream)
{
    m_stream = stream;
    m_midiData = m_stream->initData;

    for (const Event& e : m_midiData.initEvents) {
        midiOutPort()->sendEvent(e);
    }

    //! TODO Add receiving and merge events from stream
}

bool MidiPortDataSender::sendEvents(tick_t fromTick, tick_t toTick)
{
    static const std::set<EventType> SKIP_EVENTS = { EventType::ME_EOT, EventType::ME_TICK1, EventType::ME_TICK2 };

    if (m_midiData.chunks.empty()) {
        return false;
    }

    auto chunkIt = m_midiData.chunks.upper_bound(fromTick);
    --chunkIt;

    const Chunk& chunk = chunkIt->second;
    auto pos = chunk.events.lower_bound(fromTick);

    while (1) {
        const Chunk& curChunk = chunkIt->second;
        if (pos == curChunk.events.end()) {
            ++chunkIt;
            if (chunkIt == m_midiData.chunks.end()) {
                break;
            }

            const Chunk& nextChunk = chunkIt->second;
            if (nextChunk.events.empty()) {
                break;
            }

            pos = nextChunk.events.begin();
        }

        if (pos->first >= toTick) {
            break;
        }

        const Event& event = pos->second;

        if (SKIP_EVENTS.find(event.type) == SKIP_EVENTS.end()) {
            midiOutPort()->sendEvent(event);
        }

        ++pos;
    }

    return true;
}
