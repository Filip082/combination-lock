#!/usr/bin/env python3
import argparse
import struct
import yaml
import serial

HEADER = b'DOMOFON\x00'
CONFIG_VERSION = 1

# Protokół: 0xAA 0x55 <length u16 LE> <payload>
FRAME_MAGIC = b'\xAA\x55'


def pack_config(cfg) -> bytes:
    return HEADER + struct.pack('<BBBBHH',
        CONFIG_VERSION,
        cfg['log_capacity'],
        0,  # log_head
        0,  # log_count
        cfg['min_address'],
        cfg['max_address'],
    )


def pack_flat(f) -> bytes:
    pin = int(str(f['pin']), 16)  # cyfry z YAML czytane jako hex: 1234 -> 0x1234
    return struct.pack('<HH',
        f['address'],
        pin,
    ) + bytes(12)  # last_access[6] + last_denied[6] = zera


def build_payload(doc) -> bytes:
    cfg = doc['config']
    flats = doc.get('flats', [])

    min_addr = cfg['min_address']
    max_addr = cfg['max_address']

    flat_by_addr = {f['address']: f for f in flats}
    for addr in range(min_addr, max_addr + 1):
        if addr not in flat_by_addr:
            flat_by_addr[addr] = {'address': addr, 'pin': 0}

    payload = pack_config(cfg)
    for addr in range(min_addr, max_addr + 1):
        payload += pack_flat(flat_by_addr[addr])

    return payload


def send(port: str, baud: int, payload: bytes, payload_type: str = 'W'):
    frame = FRAME_MAGIC + payload_type.encode('ascii') + struct.pack('<H', len(payload)) + payload
    print(f"Wysyłam ramkę ({len(frame)} B) na {port} @ {baud} bps...")

    with serial.Serial(port, baud, timeout=3) as ser:
        ser.write(frame)
        print(f"Wysłano {len(frame)} bajtów, czekam na ACK...")
        ack = ser.read(1)
        if ack == b'\x06':
            print("OK — konfiguracja wgrana.")
        else:
            raise RuntimeError(f"Brak ACK, odebrano: {ack!r}")
        
        while True:
            line = ser.readline()
            if not line:
                break
            print(f"Urządzenie: {line.decode().rstrip()}")

def send_time(port: str, baud: int):
    import datetime
    now = datetime.datetime.now()
    date_str = now.strftime("%b %d %Y\x00")  # e.g., "Jun 10 2024"
    time_str = now.strftime("%H:%M:%S\x00")   # e.g., "14:30:00"
    print(f"Wgrywam czas do RTC: {date_str} {time_str}")
    payload = date_str.encode('ascii') + time_str.encode('ascii')
    print(f"Payload czasu ({len(payload)} B): {payload!r}")
    send(port, baud, payload, 'T')

def main():
    ap = argparse.ArgumentParser(description="Wgraj konfigurację domofonowego zamka przez UART")
    ap.add_argument('yaml_file', help="Plik konfiguracyjny YAML")
    ap.add_argument('--port', default='/dev/ttyACM0')
    ap.add_argument('--baud', type=int, default=115200)
    ap.add_argument('--dry-run', action='store_true', help="Pokaż payload bez wysyłania")
    ap.add_argument('--time', action='store_true', help="Wgraj czas kompilacji do RTC")
    args = ap.parse_args()

    if args.time:
        send_time(args.port, args.baud)
        return

    with open(args.yaml_file) as f:
        doc = yaml.safe_load(f)

    payload = build_payload(doc)

    if args.dry_run:
        print(f"Payload ({len(payload)} B):")
        print(' '.join(f'{b:02X}' for b in payload))
        return

    send(args.port, args.baud, payload)


if __name__ == '__main__':
    main()
