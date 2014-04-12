/*
 * Copyright (C) 2013 Tobias Lorenz.
 * Contact: tobias.lorenz@gmx.net
 *
 * This file is part of Tobias Lorenz's Toolkit.
 *
 * Commercial License Usage
 * Licensees holding valid commercial licenses may use this file in
 * accordance with the commercial license agreement provided with the
 * Software or, alternatively, in accordance with the terms contained in
 * a written agreement between you and Tobias Lorenz.
 *
 * GNU General Public License 3.0 Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl.html.
 */

#pragma once

#include "platform.h"

#include "VectorTypes.h"
#include "ObjectHeader.h"

#include "vector_blf_export.h"

namespace Vector {
namespace BLF {

/**
 * @brief AFDX_FRAME
 *
 * AFDX frame.
 */
class AfdxFrame : public ObjectHeader
{
public:
    AfdxFrame();

    /**
     * Ethernet (MAC) address of source computer
     * (network byte order).
     */
    BYTE sourceAddress[6];

    /**
     * The channel of the frame.
     */
    WORD channel;

    /**
     * Ethernet (MAC) address of target computer
     * (network byte order).
     */
    BYTE destinationAddress[6];

    /**
     * @brief Direction flag
     */
    enumclass Dir : WORD {
        Rx = 0,
        Tx = 1,
        TxRq = 2
    } dir ;

    /**
     * EtherType which indicates protocol for
     * Ethernet payload data
     *
     * See Ethernet standard specification for valid
     * values.
     */
    WORD type;

    /**
     * TPID when VLAN tag valid, zweo when no
     * VLAN. See Ethernet stnadard specification.
     */
    WORD tpid;

    /**
     * TCI when VLAND tag valid, zero when no
     * VLAN. See Ethernet standard specification.
     */
    WORD tci;

    /**
     * Channel number of the underlying Ethernet
     * interface, where the frame originated from.
     */
    BYTE ethChannel;

    /**
     * Status- and error flags as:
     *
     * - Bit 0: Frame from line-B
     * - Bit 1: Packet is redundant
     * - Bit 2: Frame is a fragment only
     * - Bit 3: Frame is already reassembled
     * - Bit 4: Packet is not a valid AFDX frame
     * - Bit 5: AFDX-SequenceNo is invalud
     * - Bit 6: Redundancy timeout violated
     * - Bit 7: Redundancy error encountered
     * - Bit 8: A / B interface mismatch
     * - Bit 11: Fragmentation error
     */
    WORD afdxFlags;

    /**
     * Time period since last received frame on this
     * virtual link in micro-seconds
     */
    ULONG bagUsec;

    /**
     * @brief Number of valid payLoad bytes
     *
     * Length of Ethernet payload data in bytes. Max.
     * 1500 Bytes (without Ethernet header)
     */
    WORD payLoadLength;

    /**
     * @brief Ethernet payload data
     *
     * Ethernet payload data (without Ethernet
     * header). Max 1500 data bytes per frame
     */
    BYTE * payLoad;
};

}
}