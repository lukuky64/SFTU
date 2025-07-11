#!/usr/bin/python3
# -*- encoding: utf-8 -*-

import argparse
import serial
import sys
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import os

from datetime import datetime
from argparse import RawTextHelpFormatter

# number of samples in each scanline
SCAN_WIDTH = 33

# scanline Serial start/end markers
SCAN_MARK_START = 'SCAN '
SCAN_MARK_FREQ = 'FREQ '
SCAN_MARK_END = ' END'

# output path
OUT_PATH = 'out'

# default settings
DEFAULT_BAUDRATE = 115200
DEFAULT_COLOR_MAP = 'viridis'
DEFAULT_SCAN_LEN = 2000
DEFAULT_RSSI_OFFSET = -11

# Print iterations progress
# from https://stackoverflow.com/questions/3173320/text-progress-bar-in-terminal-with-block-characters
def printProgressBar (iteration, total, prefix = '', suffix = '', decimals = 1, length = 50, fill = '█', printEnd = "\r"):
    """
    Call in a loop to create terminal progress bar
    @params:
        iteration   - Required  : current iteration (Int)
        total       - Required  : total iterations (Int)
        prefix      - Optional  : prefix string (Str)
        suffix      - Optional  : suffix string (Str)
        decimals    - Optional  : positive number of decimals in percent complete (Int)
        length      - Optional  : character length of bar (Int)
        fill        - Optional  : bar fill character (Str)
        printEnd    - Optional  : end character (e.g. "\r", "\r\n") (Str)
    """
    percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
    filledLength = int(length * iteration // total)
    bar = fill * filledLength + '-' * (length - filledLength)
    print(f'\r{prefix} |{bar}| {percent}% {suffix}', end = printEnd)
    if iteration == total: 
        print()


def main():
    parser = argparse.ArgumentParser(formatter_class=RawTextHelpFormatter, description='''
        RadioLib SX126x_Spectrum_Scan plotter script. Displays output from SX126x_Spectrum_Scan example
        as grayscale and 

        Depends on pyserial and matplotlib, install by:
        'python3 -m pip install pyserial matplotlib'

        Step-by-step guide on how to use the script:
        1. Upload the SX126x_Spectrum_Scan example to your Arduino board with SX1262 connected.
        2. Run the script with appropriate arguments.
        3. Once the scan is complete, output files will be saved to out/
    ''')
    parser.add_argument('port',
        type=str,
        help='COM port to connect to the device')
    parser.add_argument('--speed',
        default=DEFAULT_BAUDRATE,
        type=int,
        help=f'COM port baudrate (defaults to {DEFAULT_BAUDRATE})')
    parser.add_argument('--map',
        default=DEFAULT_COLOR_MAP,
        type=str,
        help=f'Matplotlib color map to use for the output (defaults to "{DEFAULT_COLOR_MAP}")')
    parser.add_argument('--len',
        default=DEFAULT_SCAN_LEN,
        type=int,
        help=f'Number of scanlines to record (defaults to {DEFAULT_SCAN_LEN})')
    parser.add_argument('--offset',
        default=DEFAULT_RSSI_OFFSET,
        type=int,
        help=f'Default RSSI offset in dBm (defaults to {DEFAULT_RSSI_OFFSET})')
    parser.add_argument('--freq',
        default=-1,
        type=float,
        help=f'Default starting frequency in MHz')
    args = parser.parse_args()

    freq_mode = False
    scan_len = args.len
    if (args.freq != -1):
        freq_mode = True
        scan_len = 2000

    # create the color map and the result array
    arr = np.zeros((SCAN_WIDTH, scan_len))

    # scanline counter
    row = 0

    # list of frequencies in frequency mode
    freq_list = []

    # open the COM port
    with serial.Serial(args.port, args.speed, timeout=10) as com:  # Add 10 second timeout
        print(f"Connected to {args.port}, collecting {scan_len} scans...")
        
        consecutive_errors = 0
        max_consecutive_errors = 50  # Increase tolerance for longer scans
        last_successful_row = 0
        lines_read = 0
        
        while(True):
            # update the progress bar
            if not freq_mode:
                printProgressBar(row, scan_len, prefix='Progress:', suffix=f'({row}/{scan_len})')

            # Log progress every 100 successful scans
            if row > 0 and row % 100 == 0 and row != last_successful_row:
                print(f"\n[{row}] Still collecting data... (Lines read: {lines_read})")
                last_successful_row = row

            # read a single line
            try:
                line = com.readline().decode('utf-8').strip()
                lines_read += 1
                
                if not line:  # Skip empty lines
                    consecutive_errors += 1
                    if consecutive_errors > max_consecutive_errors:
                        print(f"\nStopped: Too many consecutive empty reads ({consecutive_errors})")
                        print(f"Last successful scan: {row}, Total lines read: {lines_read}")
                        break
                    continue
                consecutive_errors = 0  # Reset error counter on successful read
            except Exception as e:
                consecutive_errors += 1
                print(f"\nSerial read error: {e} (consecutive errors: {consecutive_errors})")
                if consecutive_errors > max_consecutive_errors:
                    print(f"Stopped: Too many consecutive errors")
                    print(f"Last successful scan: {row}, Total lines read: {lines_read}")
                    break
                continue

            if SCAN_MARK_FREQ in line:
                try:
                    new_freq = float(line.split(' ')[1])
                    # Only exit on frequency rollover if we're in frequency mode
                    if freq_mode and (len(freq_list) > 1) and (new_freq < freq_list[-1]):
                        break

                    freq_list.append(new_freq)
                    print('{:.3f}'.format(new_freq), end = '\r')
                    continue
                except (ValueError, IndexError) as e:
                    print(f"\nFreq parsing error: {e}")
                    continue

            # check the markers
            if (SCAN_MARK_START in line) and (SCAN_MARK_END in line):
                try:
                    # get the values and remove trailing comma if present
                    data_section = line[len(SCAN_MARK_START):-len(SCAN_MARK_END)].strip()
                    if data_section.endswith(','):
                        data_section = data_section[:-1]  # Remove trailing comma
                    
                    scanline = data_section.split(',')
                    
                    # Handle different data lengths - take first SCAN_WIDTH values
                    if len(scanline) < SCAN_WIDTH:
                        print(f"\nWarning: Expected {SCAN_WIDTH} values, got {len(scanline)} - skipping")
                        continue
                    elif len(scanline) > SCAN_WIDTH:
                        # Take only the first SCAN_WIDTH values
                        scanline = scanline[:SCAN_WIDTH]
                    
                    for col in range(SCAN_WIDTH):
                        arr[col][row] = int(scanline[col])
                    
                    # increment the row counter
                    row = row + 1

                    # check if we're done
                    if (not freq_mode) and (row >= scan_len):
                        print(f"\nCompleted {row} scans successfully!")
                        break
                        
                except (ValueError, IndexError) as e:
                    consecutive_errors += 1
                    print(f"\nScanline parsing error: {e}, line: '{line}' (consecutive errors: {consecutive_errors})")
                    if consecutive_errors > max_consecutive_errors:
                        print(f"Stopped: Too many consecutive parsing errors")
                        print(f"Last successful scan: {row}, Total lines read: {lines_read}")
                        break
                    continue
    
    print(f"Exited collection loop. Final stats: row={row}, scan_len={scan_len}, freq_mode={freq_mode}")
    
    if row == 0:
        print("ERROR: No data was collected! Check your device connection and that it's sending the expected data format.")
        return
    
    # scale to the number of scans (sum of any given scanline)
    total_sum = arr.sum()
    if total_sum > 0:
        num_samples = arr.sum(axis=0).max()  # Use max instead of first element
        if num_samples > 0:
            arr *= (num_samples/arr.max())
    else:
        print("Warning: No data collected!")

    if freq_mode:
        scan_len = len(freq_list)

    # create the figure
    fig, ax = plt.subplots()

    # display the result as heatmap
    extent = [0, scan_len, -4*(SCAN_WIDTH + 1), args.offset]
    if freq_mode:
        extent[0] = freq_list[0]
        extent[1] = freq_list[-1]
    im = ax.imshow(arr[:,:scan_len], cmap=args.map, extent=extent)
    fig.colorbar(im)

    # set some properites and show 
    timestamp = datetime.now().strftime('%y-%m-%d %H-%M-%S')
    title = f'RadioLib SX126x Spectral Scan {timestamp}'
    if freq_mode:
        plt.xlabel("Frequency [Hz]")
    else:
        plt.xlabel("Time [sample]")
    plt.ylabel("RSSI [dBm]")
    ax.set_aspect('auto')
    fig.suptitle(title)
    fig.canvas.manager.set_window_title(title)
    
    # Create output directory if it doesn't exist
    os.makedirs(OUT_PATH, exist_ok=True)
    plt.savefig(f'{OUT_PATH}/{title.replace(" ", "_")}.png', dpi=300)
    plt.show()

if __name__ == "__main__":
    main()
