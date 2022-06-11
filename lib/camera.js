"use strict";
const ptz = require("../build/Release/ptz");

class Camera {
  constructor(options) {
    this.vendorId = options.vendorId;
    this.productId = options.productId;
  }

  execute(functionName, input) {
    if (!input) {
      input = {};
    }
    input.vendorId = this.vendorId;
    input.productId = this.productId;
    return ptz[functionName](input);
  }

  getCapabilities() {
    return this.execute("getCapabilities");
  }

  getAbsoluteZoom() {
    return this.execute("getAbsoluteZoom");
  }

  absoluteZoom(zoom) {
    return this.execute("absoluteZoom", {
      zoom,
    });
  }

  getRelativeZoom() {
    return this.execute("getRelativeZoom");
  }

  relativeZoomIn(speed) {
    return this.execute("relativeZoom", {
      direction: 1,
      speed,
    });
  }

  relativeZoomOut(speed) {
    return this.execute("relativeZoom", {
      direction: -1,
      speed,
    });
  }

  relativeZoomStop(speed) {
    return this.execute("relativeZoom", {
      direction: 0,
      speed,
    });
  }

  getAbsolutePanTilt() {
    return this.execute("getAbsolutePanTilt");
  }

  absolutePanTilt(pan, tilt) {
    return this.execute("absolutePanTilt", {
      pan,
      tilt,
    });
  }

  getRelativePanTilt() {
    return this.execute("getRelativePanTilt");
  }

  relativePanTilt(panDirection, panSpeed, tiltDirection, tiltSpeed) {
    return this.execute("relativePanTilt", {
      panDirection,
      panSpeed,
      tiltDirection,
      tiltSpeed,
    });
  }
}

module.exports = Camera;
