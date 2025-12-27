#!/usr/bin/env python3
"""Read a whitespace-separated XFoil-style text file and write a tab-separated copy.

Usage:
    python process_xfoil.py "D:\\Users\\bruce\\Downloads\\xf-naca0010-il-1000000.txt"
    python process_xfoil.py in.txt -o out.tsv
"""
import argparse
from pathlib import Path


def process_file(src: Path, dst: Path) -> None:
    with src.open('r', encoding='utf-8', errors='replace') as f_in, dst.open('w', encoding='utf-8') as f_out:
        for line in f_in:
            if line.strip() == '':
                f_out.write('\n')
                continue
            parts = line.split()
            f_out.write('\t'.join(parts) + '\n')


def main():
    p = argparse.ArgumentParser(description='Convert whitespace-separated columns to tab-separated (keeps header).')
    p.add_argument('src', help='Source text file (XFoil output)')
    p.add_argument('-o', '--out', help='Output file path. Defaults to source with _tsv.txt suffix')
    args = p.parse_args()

    src = Path(args.src)
    if not src.exists():
        raise SystemExit(f"Source file not found: {src}")

    if args.out:
        dst = Path(args.out)
    else:
        dst = src.with_name(src.stem + '_tsv' + src.suffix)

    process_file(src, dst)
    print(f'Wrote tab-separated file: {dst}')


if __name__ == '__main__':
    main()
