from pathlib import Path
    
def translate(path: str | Path):
        path = Path(path)
        if not path.exists():
            raise FileNotFoundError(path)

        with path.open('r', encoding='utf-8', errors='replace') as fh:
            print('// Generated from', path.name)
            print('float data[][4] = {')
            # skip any leading non-data lines until header found
            header_found = False
            line_count = 0
            for raw in fh:
                line = raw.strip()
                if not line:
                    continue
                parts = line.split()
                if not header_found:
                    # look for header containing 'Alpha' and 'CL'
                    low = [p.lower() for p in parts]
                    if 'alpha' in low and 'cl' in low:
                        header_found = True
                        continue
                    else:
                        # maybe filename or title line; skip
                        continue

                # now parse numeric rows
                try:
                    a = float(parts[0])
                    c_l = float(parts[1])
                    c_d = float(parts[2])
                    c_m = float(parts[3])
                    print(f'    {{{a}f, {c_l}f, {c_d}f, {c_m}f}},')    
                    line_count += 1
                except Exception:
                    # skip malformed lines
                    continue

            print('};')
            print(f'// {line_count} data points')
            print(f'int line_count = {line_count};')


#translate('NACA0010.txt')
#translate('FX-02-196.txt')
translate('FX-60-126.txt')
