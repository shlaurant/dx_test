Copy-Item -Path "shader\*.cso" -Destination "cmake-build-debug-vs-64\bin\shader" -Recurse -force
Copy-Item -Path "resource\*" -Destination "cmake-build-debug-vs-64\bin\resource" -Recurse -force

Copy-Item -Path "shader\*.cso" -Destination "cmake-build-release-vs-64\bin\shader" -Recurse -force
Copy-Item -Path "resource\*" -Destination "cmake-build-release-vs-64\bin\resource" -Recurse -force