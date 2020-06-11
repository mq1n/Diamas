#pragma once

extern bool GetExeCRC(uint32_t & r_dwProcCRC, uint32_t & r_dwFileCRC);

extern void BuildProcessCRC();
extern uint8_t GetProcessCRCMagicCubePiece();
