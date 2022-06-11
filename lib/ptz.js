"use strict";

const ptz = require("../build/Release/ptz");
const Camera = require("./camera");

class PTZ {
  static listDevices(callback) {
    return ptz.listDevices();
  }

  static getCamera(options) {
    if (!options) {
      options = {};
    }
    options.vendorId = options.vendorId || 0;
    options.productId = options.productId || 0;
    return new Camera(options);
  }
}

module.exports = PTZ;
