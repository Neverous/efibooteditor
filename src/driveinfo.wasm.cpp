// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "driveinfo.h"

auto DriveInfo::getAll(bool) -> QVector<DriveInfo>
{
    return all;
}
