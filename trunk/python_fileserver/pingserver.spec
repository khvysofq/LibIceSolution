# -*- mode: python -*-
a = Analysis(['pingserver.py'],
             pathex=['F:\\GitHub\\trunk\\python_fileserver'],
             hiddenimports=[],
             hookspath=None,
             runtime_hooks=None)
pyz = PYZ(a.pure)
exe = EXE(pyz,
          a.scripts,
          a.binaries,
          a.zipfiles,
          a.datas,
          name='pingserver.exe',
          debug=False,
          strip=None,
          upx=True,
          console=True )
