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

#include "EthernetRxError.h"

#include <cstring>

namespace Vector {
namespace BLF {

EthernetRxError::EthernetRxError() :
    ObjectHeader(),
    structLength(),
    channel(),
    dir(),
    reserved1(),
    fcs(),
    frameDataLength(),
    reserved2(),
    error(),
    frameData(nullptr)
{
}

EthernetRxError::~EthernetRxError()
{
    delete[] frameData;
    frameData = nullptr;
}

char * EthernetRxError::read(char * buffer)
{
    size_t size;

    // previous data
    buffer = ObjectHeader::read(buffer);

    // structLength
    size = sizeof(structLength);
    memcpy((void *) &structLength, buffer, size);
    buffer += size;

    // channel
    size = sizeof(channel);
    memcpy((void *) &channel, buffer, size);
    buffer += size;

    // dir
    size = sizeof(dir);
    memcpy((void *) &dir, buffer, size);
    buffer += size;

    // reserved1
    size = sizeof(reserved1);
    memcpy((void *) &reserved1, buffer, size);
    buffer += size;

    // fcs
    size = sizeof(fcs);
    memcpy((void *) &fcs, buffer, size);
    buffer += size;

    // frameDataLength
    size = sizeof(frameDataLength);
    memcpy((void *) &frameDataLength, buffer, size);
    buffer += size;

    // reserved2
    size = sizeof(reserved2);
    memcpy((void *) &reserved2, buffer, size);
    buffer += size;

    // error
    size = sizeof(error);
    memcpy((void *) &error, buffer, size);
    buffer += size;

    // frameData
    size = frameDataLength;
    frameData = new char[size];
    memcpy((void *) frameData, buffer, size);
    buffer += size;

    return buffer;
}

char * EthernetRxError::write(char * buffer)
{
    // @todo
}

size_t EthernetRxError::calculateObjectSize()
{
    size_t size =
        ObjectHeader::calculateObjectSize() +
        sizeof(structLength) +
        sizeof(channel) +
        sizeof(dir) +
        sizeof(reserved1) +
        sizeof(fcs) +
        sizeof(frameDataLength) +
        sizeof(reserved2) +
        sizeof(error) +
        frameDataLength;

    return size;
}

}
}
