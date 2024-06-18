# FAST Computer Vision

A computer vision application to track ArUco markers.

**What is FAST?**

<ins>F</ins>lexible, <ins>A</ins>ccessible <ins>S</ins>trategies for <ins>T</ins>imely Digital Exhibit Design

Museums play a critical role in engaging their communities around urgent 
issues that emerge in the public sphere. However, typical exhibit development 
timelines can stretch for years, and "what's new" can change swiftly before 
an exhibition is even launched. What if museums could stay agile and dynamic, 
changing out content as community needs shift, science progresses, and the 
world changes? What if timely exhibit offerings could be not just efficiently 
produced, but accessible and welcoming to all visitors? This vision of 
efficiency and accessibility is at the center of FAST.

For more information about FAST, please see:

* [FAST Booklet](https://mos.widen.net/s/gkftj8tgl8/fast_booklet), an 
introduction and explanation of the FAST project
* [FAST Toolkit](https://mos.widencollective.com/portals/zh7gauqj/FASTPublicPortal), 
a portal for all assets and documentation published from the FAST project

## System Requirements

### Runtime Requirements

* Microsoft Windows 10 or 11
* Microsoft Visual C++ 2015-2022 Redistributable (x64)
* Teledyne FLIR machine vision camera

Additional hardware is required to operate some FAST user experiences. 
See FAST production documentation for more details.

### Build Requirements

* Qt 6.5.0
* OpenCV 4.6.0
* FLIR Spinnaker C++ SDK 2.5.0
* Microsoft Visual C++ Compiler 16.11 (MSVC2019 64-bit)

## Installation

1. Download the most recent version of the application from 
[Releases](https://github.com/FAST-Digital-Exhibit-Design/FAST-Computer-Vision/releases)
2. Extract the ZIP file where you want the application installed
3. Open **fast-computer-vision.exe** to run the application

## Documentation

See the [User Manual](fast-computer-vision/doc/UserManual.md)

## Contributions

This repo is only maintained with bug fixes and Pull Requests are not accepted 
at this time. If you'd like to contribute, please post questions and 
comments about using FAST Computer Vision to 
[Discussions](https://github.com/FAST-Digital-Exhibit-Design/FAST-Computer-Vision/discussions) 
and report bugs using [Issues](https://github.com/FAST-Digital-Exhibit-Design/FAST-Computer-Vision/issues).

## Citation

If you reference this software in a publication, please cite it as follows:

**APA**
```
Museum of Science, Boston. FAST Computer Vision [Computer software]. https://github.com/FAST-Digital-Exhibit-Design/FAST-Computer-Vision
```

**BibTeX**
```
@software{Museum_of_Science_FAST_Computer_Vision,
author = {{Museum of Science, Boston}},
license = {GPL-3.0-only},
title = {{FAST Computer Vision}},
url = {https://github.com/FAST-Digital-Exhibit-Design/FAST-Computer-Vision}
}
```

## Notices

Copyright (C) 2024 Museum of Science, Boston
<https://www.mos.org/>

This program was developed through a grant to the Museum of Science, 
Boston from the Institute of Museum and Library Services under Award 
#MG-249646-OMS-21. For more information about this grant, see 
<https://www.imls.gov/grants/awarded/mg-249646-oms-21>.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the GNU General Public License is provided in the [LICENSES](LICENSES) 
folder. For more information about this license, see 
<https://www.gnu.org/licenses/gpl-3.0.html>.

`SPDX-License-Identifier: GPL-3.0-only`

A portion of the User Manual documentation redistributes parameter 
descriptions from the OpenCV ArUco module documentation licensed under 
the Apache License, Version 2.0. A copy of the Apache License, Version 2.0 
is provided in the [LICENSES](LICENSES) folder. For more information about 
this license, see <http://www.apache.org/licenses/LICENSE-2.0>.

`SPDX-License-Identifier: Apache-2.0`

Third party copyrights and trademarks are property of their respective 
owners.

### The Qt Framework

This program was developed using The Qt Framework with the Qt Community 
Open Source License under the terms of the GNU General Public License as 
published by the Free Software Foundation. A copy of the Qt source code 
is available at <https://github.com/FAST-Digital-Exhibit-Design/FAST-Qt> 
and a copy of the GNU General Public License is provided in the 
[LICENSES](LICENSES) folder. For more information about this license, see 
<https://www.gnu.org/licenses/gpl-3.0.html>.

The Qt Company Ltd and its subsidiaries is the owner of Qt trademarks 
worldwide. The Qt Framework is Copyright (C) 2008-2024 The Qt Company 
Ltd.

### OpenCV

This program was developed using OpenCV (Open Source Computer Vision 
Library) under the terms of the Apache License, Version 2.0 as 
published by the The Apache Software Foundation. A copy of the Apache 
License, Version 2.0  is provided in the [LICENSES](LICENSES) folder. 
For more information about this license, see 
<http://www.apache.org/licenses/LICENSE-2.0>.

OpenCV is the owner of OpenCV trademarks in the United States. OpenCV is 
Copyright (C) 2024 OpenCV team.

### FLIR Spinnaker SDK

This program was developed using the FLIR Spinnaker SDK under the terms 
of the FLIR Spinnaker SDK License Agreement.

Teledyne FLIR LLC is the owner of FLIR Spinnaker SDK trademarks in the 
United States. FLIR Spinnaker SDK is Copyright (C) 2001-2024 Teledyne 
FLIR LLC.