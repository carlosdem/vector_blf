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
 * @brief APP_TRIGGER
 *
 * Application defined trigger to be saved in BLF log file (currently not used in CANoe
 * / CANalyzer).
 */
class AppTrigger : public ObjectHeader
{
public:
    AppTrigger();

    /**
     * @brief pre-trigger time
     *
     * Pre trigger time.
     */
    ULONGLONG preTriggerTime;

    /**
     * @brief post-trigger time
     *
     * Post trigger time.
     */
    ULONGLONG postTriggerTime;

    /**
     * @brief channel of event which triggered (if any)
     *
     * Trigger that channel belongs to.
     */
    WORD channel;

    /**
     * @brief trigger type
     */
    enumclass Flags : WORD {
        SingleTrigger = 0x0000, /**< single trigger type */
        LoggingStart = 0x0001, /**< start of logging trigger type */
        LoggingStop = 0x0002 /**< stop of logging trigger type */
    } flags;

    /**
     * @brief app specific member 2
     *
     * Reserved.
     */
    DWORD appSpecific2;
};

}
}