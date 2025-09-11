mkdir -p payload/usr/local/bin

cp build/ardan payload/usr/local/bin/

pkgbuild --root payload \
  --identifier com.ardan.ardanlang \
  --version 1.0.0 \
  --install-location / \
  ardan.pkg
