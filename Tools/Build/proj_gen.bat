@echo off


pyProjectCreator.py -r ../.. -n MatrixPilot -t SIL -d USE_FREERTOS -c Config/Cessna
pyProjectCreator.py -r ../.. -n MatrixPilot -t SIL -d USE_FREERTOS

pyProjectCreator.py -r ../.. -n MatrixPilot -t PX4 -c Config/Cessna
pyProjectCreator.py -r ../.. -n MatrixPilot -t UDB4 -d USE_FREERTOS
pyProjectCreator.py -r ../.. -n MatrixPilot -t UDB5 -d USE_FREERTOS
pyProjectCreator.py -r ../.. -n MatrixPilot -t AUAV3 -d USE_FREERTOS

pyProjectCreator.py -r ../.. -n RollPitchYaw -t PX4
pyProjectCreator.py -r ../.. -n RollPitchYaw -t UDB4
pyProjectCreator.py -r ../.. -n RollPitchYaw -t UDB5
pyProjectCreator.py -r ../.. -n RollPitchYaw -t AUAV3

pyProjectCreator.py -r ../.. -n LedTest -t PX4
pyProjectCreator.py -r ../.. -n LedTest -t UDB4
pyProjectCreator.py -r ../.. -n LedTest -t UDB5
pyProjectCreator.py -r ../.. -n LedTest -t AUAV3

goto END:
:END

pause
