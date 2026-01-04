
cd ..

pio run -e cyd_tft
pio run -t buildfs -e cyd_tft

cd firmware_single_file
call make_generic_cyd_tft v370