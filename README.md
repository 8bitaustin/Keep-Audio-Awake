# keep-audio-awake

My Legion desktop kept putting the audio device to sleep, which caused popping and crackling whenever audio first started playing. This happened on both Windows and Linux, but this is a fix specifically for Linux. It's a small program you can run as a systemd service to keep the audio channel open and prevent the device from sleeping.

---

## The Problem

On Linux systems with an NVIDIA GPU, the HDMI audio device is tied to the GPU's power state. When no audio is playing, the system suspends the audio device to save power. This causes:

- An audible **pop or crackle** at the start of every audio stream
- A brief **delay** before audio begins playing
- An audible **click** when audio stops and the device goes back to sleep

This affects any application that plays audio through the NVIDIA HDMI output — browsers, media players, games, etc.

You might try the usual advice of setting `power_save=0` in `/etc/modprobe.d/` or poking the PCI device's sysfs power control — these don't work. The suspend is happening at the GPU/PipeWire level, not the ALSA level, so those fixes never reach the right place.

---

## The Solution

`keep_audio_awake` plays a continuous stream of silence through PipeWire. This keeps the audio device active at all times without making any sound and without blocking other apps from using it.

---

## Requirements

- Linux with PipeWire (tested on CachyOS / Arch with PipeWire 1.6.1)
- NVIDIA GPU with HDMI audio
- `libpulse` (PulseAudio client library — works transparently with PipeWire)

To confirm your setup matches:

```bash
# Should show "HDA NVidia" as one of the cards
cat /proc/asound/cards

# Should show "PulseAudio (on PipeWire ...)"
pactl info | grep "Server Name"
```

---

## Option A — Use the Precompiled Binary

Download `keep_audio_awake` from the releases and skip to the **Auto-start** section below.

---

## Option B — Build from Source

Install the build dependency:

```bash
# Arch / CachyOS / Manjaro
sudo pacman -S libpulse

# Ubuntu / Debian
sudo apt install libpulse-dev
```

Compile:

```bash
gcc keep_audio_awake.c -o keep_audio_awake -lpulse-simple -lpulse
```

Run it manually to test:

```bash
./keep_audio_awake
```

Play something in your browser or media player — the popping should be gone. Press `Ctrl+C` to stop.

---

## Auto-start on Login (KDE / systemd)

Once you've confirmed it works, set it up as a service so it starts automatically with your desktop session.

**1. Put the binary somewhere permanent:**

```bash
mkdir -p ~/.local/bin
cp keep_audio_awake ~/.local/bin/keep_audio_awake
```

**2. Create the service file:**

```bash
mkdir -p ~/.config/systemd/user
nano ~/.config/systemd/user/keep-audio-awake.service
```

Paste this into the file:

```ini
[Unit]
Description=Keep audio device awake
After=pipewire.service pipewire-pulse.service
Wants=pipewire.service pipewire-pulse.service

[Service]
ExecStart=%h/.local/bin/keep_audio_awake
Restart=on-failure
RestartSec=5

[Install]
WantedBy=default.target
```

**3. Enable and start it:**

```bash
systemctl --user daemon-reload
systemctl --user enable --now keep-audio-awake.service
```

**4. Check it's running:**

```bash
systemctl --user status keep-audio-awake.service
```

The service will start automatically after PipeWire is ready on every login and restart itself if it ever crashes.

---

## How It Works

The program uses the PulseAudio simple API (`libpulse-simple`) to open a playback stream on the default audio sink. It writes 100ms chunks of zeroed (silent) audio in a loop. Because it goes through PipeWire rather than talking directly to the ALSA hardware, it shares the device normally with everything else — no exclusive locking, no interference.

---

## License

MIT
