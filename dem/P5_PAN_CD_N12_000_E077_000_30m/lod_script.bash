#!/bin/bash

# --- CONFIGURATION ---
SOURCE_FILE="P5_PAN_CD_N12_000_E077_000_DEM_30m.tif"
RESAMPLING_METHOD="bilinear"
OUTPUT_BASENAME="elevation_lod"

# --- STAGE 1: Get the dimensions of the original LOD 0 texture ---
# This determines the master size for all output files.
MASTER_SIZE=$(gdalinfo ${SOURCE_FILE} | grep "Size is" | cut -d' ' -f3,4 --output-delimiter=' ')
MASTER_WIDTH=$(echo ${MASTER_SIZE} | cut -d' ' -f1)
MASTER_HEIGHT=$(echo ${MASTER_SIZE} | cut -d' ' -f2)

echo "Master texture size for all outputs: ${MASTER_WIDTH} x ${MASTER_HEIGHT}"
echo "----------------------------------------"


# --- STAGE 2: Generate correctly downscaled temporary files ---
# We create temporary GeoTIFFs because they are a robust intermediate format.
echo "Generating downscaled LOD source files (as temporary GeoTIFFs)..."
for i in {0..4}; do
  scale=$((100 / (2**i)))
  if [ $scale -eq 0 ]; then scale=1; fi

  echo "Creating temp_lod${i}.tif at ${scale}% scale..."
  gdal_translate -of GTiff -outsize ${scale}% ${scale}% -r ${RESAMPLING_METHOD} ${SOURCE_FILE} "temp_lod${i}.tif"
done
echo "----------------------------------------"


# --- STAGE 3: Upscale temps and convert to final ENVI .raw/.hdr format ---
# This is the key stage. We read each temp file, resize it to the master size,
# and write it out in the ENVI format your application needs.
echo "Upscaling LODs and converting to final ENVI (.raw/.hdr) format..."
for i in {0..4}; do
  FINAL_FILENAME="${OUTPUT_BASENAME}${i}"
  echo "Processing final ${FINAL_FILENAME}.raw/.hdr..."
  
  gdal_translate \
    -of ENVI \
    -ot Float32 \
    -outsize ${MASTER_WIDTH} ${MASTER_HEIGHT} \
    -r ${RESAMPLING_METHOD} \
    "temp_lod${i}.tif" \
    "${FINAL_FILENAME}.raw" # Note: No extension here, GDAL creates .raw and .hdr

done
echo "----------------------------------------"


# --- STAGE 4: Clean up temporary files ---
echo "Cleaning up temporary GeoTIFF files..."
rm temp_lod*.tif

echo "SUCCESS: All LOD files have been generated."
echo "You should now have 5 pairs of ${OUTPUT_BASENAME}*.raw and ${OUTPUT_BASENAME}*.hdr files."
echo "The dimensions in each .hdr file should be identical (${MASTER_WIDTH}x${MASTER_HEIGHT})."