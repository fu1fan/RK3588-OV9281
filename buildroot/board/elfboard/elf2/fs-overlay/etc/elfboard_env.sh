#!/bin/sh

/usr/sbin/alsactl restore

# Clear inherited legacy settings before selecting independent output timing.
cat > /etc/profile.d/weston.sh <<'EOF'
export QT_QPA_PLATFORM=wayland
export XDG_RUNTIME_DIR=/var/run
unset WESTON_DISABLE_ATOMIC WESTON_DRM_MIRROR
export WESTON_DRM_PRIMARY=HDMI-A-1
EOF

# Mirroring remains an explicit opt-in; mixed refresh rates use independent
# outputs by default. A missing bootloader variable must not enable mirroring.
DISP=$(fw_printenv -n disp_type 2>/dev/null) || DISP=
if [ "$DISP" = "sync" ]; then
    echo 'export WESTON_DRM_MIRROR=1' >> /etc/profile.d/weston.sh
fi
