# Project Terraria


## To Dos
### Part I
- [x] make a badass triangle
- [x] ez one stop cmake setup
- [x] refactoring main
- [x] camera view
- [ ] DEM loader
- [ ] render basic terrain(flat)
- [ ] start with lod work

### Part II
- [ ] road to terrain
- [ ] buildings to terrain

### Presentation
- [ ] make paper presentation slide template
- [ ] motivation
- [ ] Key ideas
- [ ] Basic render demo


    


## Setup

```bash
# for dawn build
cmake -B build 
cmake --build build

# for web build 
ecmake cmake --build build-web
cmake --build build-web

#app.html is created at the build-web needs to be render can use Live-Server vscode extenstion or some basic server setup

```

## 📥 DEM (Digital Elevation Model)

### Example Sources
- [NRSC Bhuvan (India)](https://bhuvan.nrsc.gov.in/)
- [USGS Earth Explorer](https://earthexplorer.usgs.gov/)
- [SRTM or ASTER DEM repositories](https://lpdaac.usgs.gov/products/)
Use GDAL to inspect file properties:
```
sudo apt update
sudo apt install libgal-dev gdal-bin
#libgal fr the loader part and gdal bin is the cli tool
```
### check dem file
```
 gdalinfo P5_PAN_CD_N12_000_E077_000_30m/P5_PAN_CD_N12_000_E077_000_DEM_30m.tif
```
### Output
```


    Driver: GTiff/GeoTIFF
    Files: P5_PAN_CD_N12_000_E077_000_30m/P5_PAN_CD_N12_000_E077_000_DEM_30m.tif
    Size is 3600, 3600
    Coordinate System is:
    GEOGCRS["WGS 84",
        ENSEMBLE["World Geodetic System 1984 ensemble",
            MEMBER["World Geodetic System 1984 (Transit)"],
            MEMBER["World Geodetic System 1984 (G730)"],
            MEMBER["World Geodetic System 1984 (G873)"],
            MEMBER["World Geodetic System 1984 (G1150)"],
            MEMBER["World Geodetic System 1984 (G1674)"],
            MEMBER["World Geodetic System 1984 (G1762)"],
            MEMBER["World Geodetic System 1984 (G2139)"],
            ELLIPSOID["WGS 84",6378137,298.257223563,
                LENGTHUNIT["metre",1]],
            ENSEMBLEACCURACY[2.0]],
        PRIMEM["Greenwich",0,
            ANGLEUNIT["degree",0.0174532925199433]],
        CS[ellipsoidal,2],
            AXIS["geodetic latitude (Lat)",north,
                ORDER[1],
                ANGLEUNIT["degree",0.0174532925199433]],
            AXIS["geodetic longitude (Lon)",east,
                ORDER[2],
                ANGLEUNIT["degree",0.0174532925199433]],
        USAGE[
            SCOPE["Horizontal component of 3D system."],
            AREA["World."],
            BBOX[-90,-180,90,180]],
        ID["EPSG",4326]]
    Data axis to CRS axis mapping: 2,1
    Origin = (77.000000100400001,12.999994938700000)
    Pixel Size = (0.000277778000000,-0.000277778000000)
    Metadata:
    AREA_OR_POINT=Area
    TIFFTAG_ARTIST=PCI
    TIFFTAG_SOFTWARE=PCI Geomatica
    Image Structure Metadata:
    INTERLEAVE=BAND
    Corner Coordinates:
    Upper Left  (  77.0000001,  12.9999949) ( 77d 0' 0.00"E, 12d59'59.98"N)
    Lower Left  (  77.0000001,  11.9999941) ( 77d 0' 0.00"E, 11d59'59.98"N)
    Upper Right (  78.0000009,  12.9999949) ( 78d 0' 0.00"E, 12d59'59.98"N)
    Lower Right (  78.0000009,  11.9999941) ( 78d 0' 0.00"E, 11d59'59.98"N)
    Center      (  77.5000005,  12.4999945) ( 77d30' 0.00"E, 12d29'59.98"N)
    Band 1 Block=3600x1 Type=Float32, ColorInterp=Gray
    Description = Geocoded DEM channel
    NoData Value=-32768

```