Copy-Item -Path "shader\debug\*.cso" -Destination "cmake-build-debug-vs-64\bin\shader" -Recurse -force
Copy-Item -Path "resource\*" -Destination "cmake-build-debug-vs-64\bin\resource" -Recurse -force

Copy-Item -Path "shader\release\*.cso" -Destination "cmake-build-release-vs-64\bin\shader" -Recurse -force
Copy-Item -Path "resource\*" -Destination "cmake-build-release-vs-64\bin\resource" -Recurse -force