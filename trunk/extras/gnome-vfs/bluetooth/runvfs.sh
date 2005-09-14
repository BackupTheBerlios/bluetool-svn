cfg=/etc/gnome-vfs-2.0/modules/bluetooth.conf 
if [ ! -e ${cfg} ]; then
echo "you have to copy the configuration file to ${cfg} before using the module"
else
export PYTHONPATH=$(pwd):$PYTHONPATH
mkdir /tmp/bluetoothvfs
export TMPDIR=/tmp/bluetoothvfs
nautilus --no-desktop bluetooth:///
fi
