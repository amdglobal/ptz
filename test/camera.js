"use strict";

const ptz = require("../lib/ptz");

var _camera;
var _capabilities;

describe("camera", () => {
  before(() => {
    _camera = ptz.getCamera();
    _capabilities = _camera.getCapabilities();
  });

  it("got camera", () => {
    expect(_capabilities).not.toBeUndefined();
  });

  it("getCapabilities promise", () => {
    const capabilities = _camera.getCapabilities();
    expect(capabilities).not.toBeUndefined();
    expect(capabilities).toHaveProperty("absoluteZoom");
    expect(capabilities).toHaveProperty("relativeZoom");
    expect(capabilities).toHaveProperty("absolutePanTilt");
    expect(capabilities).toHaveProperty("relativePanTilt");
    expect(capabilities).toHaveProperty("absoluteRoll");
    expect(capabilities).toHaveProperty("relativeRoll");
  });

  it("getAbsoluteZoom", () => {
    if (_capabilities.absoluteZoom) {
      const zoomInfo = _camera.getAbsoluteZoom();
      expect(zoomInfo).not.toBeUndefined();
      expect(zoomInfo).toHaveProperty("min");
      expect(zoomInfo).toHaveProperty("max");
      expect(zoomInfo).toHaveProperty("resolution");
      expect(zoomInfo).toHaveProperty("current");
      expect(zoomInfo).toHaveProperty("default");
    }
  });

  it("absoluteZoom", () => {
    if (_capabilities.absoluteZoom) {
      return _camera.absoluteZoom(0);
    }
  });

  it("getRelativeZoom", () => {
    if (_capabilities.relativeZoom) {
      const zoomInfo = _camera.getRelativeZoom();
      expect(zoomInfo).not.toBeUndefined();
      expect(zoomInfo).toHaveProperty("direction");
      expect(zoomInfo).toHaveProperty("digitalZoom");
      expect(zoomInfo).toHaveProperty("minSpeed");
      expect(zoomInfo).toHaveProperty("maxSpeed");
      expect(zoomInfo).toHaveProperty("resolutionSpeed");
      expect(zoomInfo).toHaveProperty("currentSpeed");
    }
  });

  it("relativeZoomIn", () => {
    if (_capabilities.relativeZoom) {
      return _camera.relativeZoomIn(1);
    }
  });

  it("relativeZoomOut", () => {
    if (_capabilities.relativeZoom) {
      return _camera.relativeZoomOut(1);
    }
  });

  it("relativeZoomStop", () => {
    if (_capabilities.relativeZoom) {
      return _camera.relativeZoomStop(1);
    }
  });

  it("getAbsolutePanTilt", () => {
    if (_capabilities.absolutePanTilt) {
      const panTilt = _camera.getAbsolutePanTilt();
      expect(panTilt).not.toBeUndefined();
      expect(panTilt).toHaveProperty("minPan");
      expect(panTilt).toHaveProperty("minTilt");
      expect(panTilt).toHaveProperty("maxPan");
      expect(panTilt).toHaveProperty("maxTilt");
      expect(panTilt).toHaveProperty("resolutionPan");
      expect(panTilt).toHaveProperty("resolutionTilt");
      expect(panTilt).toHaveProperty("currentPan");
      expect(panTilt).toHaveProperty("currentTilt");
      expect(panTilt).toHaveProperty("defaultPan");
      expect(panTilt).toHaveProperty("defaultTilt");
    }
  });

  it("absolutePanTilt", () => {
    if (_capabilities.absolutePanTilt) {
      return _camera.absolutePanTilt(0, 0);
    }
  });

  it("getRelativePanTilt", () => {
    if (_capabilities.relativePanTilt) {
      const panTilt = _camera.getRelativePanTilt();
      expect(panTilt).not.toBeUndefined();
      expect(panTilt).toHaveProperty("panDirection");
      expect(panTilt).toHaveProperty("tiltDirection");
      expect(panTilt).toHaveProperty("minPanSpeed");
      expect(panTilt).toHaveProperty("minTiltSpeed");
      expect(panTilt).toHaveProperty("maxPanSpeed");
      expect(panTilt).toHaveProperty("maxTiltSpeed");
      expect(panTilt).toHaveProperty("resolutionPanSpeed");
      expect(panTilt).toHaveProperty("resolutionTiltSpeed");
      expect(panTilt).toHaveProperty("defaultPanSpeed");
      expect(panTilt).toHaveProperty("defaultTiltSpeed");
      expect(panTilt).toHaveProperty("currentPanSpeed");
      expect(panTilt).toHaveProperty("currentTiltSpeed");
    }
  });

  it("relativePanTilt", () => {
    if (_capabilities.relativePanTilt) {
      return _camera.relativePanTilt(0, 1, 0, 1);
    }
  });
});
