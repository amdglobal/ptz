{
  "targets": [
    {
      "target_name": "ptz",
      "sources": ["lib/ptz.cpp"],
      "libraries": ["-luvc"],
      "cflags": ["-std=c++17 -g -Wno-cast-function-type"],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "/usr/include/",
        "/usr/include/libusb-1.0",
        "/usr/local/include",
        "/usr/local/include/libuvc",
        "/usr/local/Cellar/libuvc/0.0.5/include",
        "/usr/local/Cellar/libuvc/0.0.5/include/libuvc",
        "/usr/local/Cellar/libusb/1.0.21/include",
        "/usr/local/Cellar/libusb/1.0.21/include/libusb-1.0"
      ]
    }
  ]
}
