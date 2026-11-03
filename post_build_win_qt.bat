@rem // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
@rem // Author: Miao Yu (Carnegie Mellon University)
@rem // Author: Virgil Gligor (Carnegie Mellon University)

cd .\output\dist

@rem [TODO] Using a common plugin folder does not work at the moment.
@rem windeployqt --release --plugindir libs\qt\plugins sap\RDesktop.exe
@rem windeployqt --release --plugindir libs\qt\plugins sap\RFiles.exe
@rem windeployqt --release --plugindir libs\qt\plugins tools\control_monitor\control_monitor.exe

@rem [TODO] Fall back to using separate DLLs and plugins. Remove the code below after we can use a common plugin folder.
windeployqt --release sap\RDesktop.exe
windeployqt --release sap\RFiles.exe
windeployqt --release tools\control_monitor\control_monitor.exe

cd ..\..\