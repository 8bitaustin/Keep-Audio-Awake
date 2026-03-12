# keep-audio-awake
A small C program that prevents NVIDIA HDMI audio devices from sleeping on Linux, eliminating the audible pop/click and delay when audio playback begins.

# The Problem
On Linux systems with an NVIDIA GPU, the HDMI audio device is tied to the GPU's power state via vga_switcheroo. When no audio is playing, the system suspends the audio device to save power. This causes:

An audible pop or crackle at the start of every audio stream
A brief delay before audio begins playing
An audible click when audio stops and the device powers back down

This affects any application that plays audio through the NVIDIA HDMI output — browsers, media players, games, etc. Standard fixes such as setting options snd_hda_intel power_save=0 in /etc/modprobe.d/ or writing on to the PCI device's power/control sysfs entry do not solve this because the suspend is managed at the GPU/PipeWire level, not the ALSA level.

# The Solution
keep_audio_awake opens a continuous silent audio stream through PipeWire's PulseAudio interface. This keeps the audio device active at all times without playing any audible sound and without blocking other applications from using the device.

# Requirements

Linux with PipeWire (tested on CachyOS / Arch-based distros with PipeWire 1.6.1)
NVIDIA GPU with HDMI audio (HDA NVidia in cat /proc/asound/cards)
libpulse (PulseAudio client library — works transparently with PipeWire)


Building from Source
Install the dependency:
# Arch / CachyOS / Manjaro
sudo pacman -S libpulse

# Ubuntu / Debian 
sudo apt install libpulse-dev
Compile:
bashgcc keep_audio_awake.c -o keep_audio_awake -lpulse-simple -lpulse


# Running
bash/dash/fish

./keep_audio_awake

Press Ctrl+C to stop. While running, all other applications can play audio normally.

# Auto-start on Login (KDE / systemd)
To have the program start automatically with your desktop session:
1. Place the binary somewhere permanent:
bashmkdir -p ~/.local/bin
cp keep_audio_awake ~/.local/bin/keep_audio_awake
2. Create the systemd user service:
bashmkdir -p ~/.config/systemd/user
nano ~/.config/systemd/user/keep-audio-awake.service
Paste the following:
ini[Unit]
Description=Keep audio device awake
After=pipewire.service pipewire-pulse.service
Wants=pipewire.service pipewire-pulse.service

[Service]
ExecStart=%h/.local/bin/keep_audio_awake
Restart=on-failure
RestartSec=5

[Install]
WantedBy=default.target
3. Enable and start the service:
bashsystemctl --user daemon-reload
systemctl --user enable --now keep-audio-awake.service
4. Verify it is running:
bashsystemctl --user status keep-audio-awake.service
The service will start automatically after PipeWire is ready on every login and will restart itself if it crashes.

Verifying Your Setup
To confirm you have the same hardware/software configuration this was developed for:
bash# Should show HDA NVidia as card 1
cat /proc/asound/cards

# Should show PulseAudio (on PipeWire ...)
pactl info | grep "Server Name"

How It Works
The program uses the PulseAudio simple API (libpulse-simple) to open a playback stream connected to the system's default audio sink. It writes 100ms chunks of zeroed (silent) audio in a loop. Because it uses the PulseAudio/PipeWire layer rather than direct ALSA hardware access, the stream is mixed with other applications' audio transparently — no exclusive device locking occurs.

License
MIT
