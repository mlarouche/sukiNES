#include "ppudebuginfodialog.h"

// sukiNES includes
#include <ppu.h>

PPUDebugInfoDialog::PPUDebugInfoDialog(QWidget* parent)
: QDialog(parent)
{
	_ui.setupUi(this);
}

PPUDebugInfoDialog::~PPUDebugInfoDialog()
{
}

void PPUDebugInfoDialog::updatePPUInfo(sukiNES::PPU* ppu)
{
	// PPU Control
	_ui.checkPpuControl_AddressIncremnet->setChecked( (unsigned)ppu->_ppuControl.addressIncrement );
	_ui.checkPpuControl_SpritePatternTable->setChecked( (unsigned)ppu->_ppuControl.spritePatternTable );
	_ui.checkPpuControl_BackgroundPatternTable->setChecked( (unsigned)ppu->_ppuControl.backgroundPatternTable );
	_ui.checkPpuControl_SpriteSize->setChecked( (unsigned)ppu->_ppuControl.spriteSize );
	_ui.checkPpuControl_GenerateNMI->setChecked( (unsigned)ppu->_ppuControl.generateNmi );

	// PPU Mask
	_ui.checkPpuMask_Greyscale->setChecked( (unsigned)ppu->_ppuMask.greyscale );
	_ui.checkPpuMask_ShowLeftmostBackground->setChecked( (unsigned)ppu->_ppuMask.showBackgroundLeftmost );
	_ui.checkPpuMask_ShowLeftmostSprites->setChecked( (unsigned)ppu->_ppuMask.showSpritesLeftmost );
	_ui.checkPpuMask_ShowBackground->setChecked( (unsigned)ppu->_ppuMask.showBackground );
	_ui.checkPpuMask_ShowSprites->setChecked( (unsigned)ppu->_ppuMask.showSprites );
	_ui.checkPpuMask_IntensifyReds->setChecked( (unsigned)ppu->_ppuMask.intensifyRed );
	_ui.checkPpuMask_IntensifyGreens->setChecked( (unsigned)ppu->_ppuMask.intensifyGreen );
	_ui.checkPpuMask_IntensifyBlues->setChecked( (unsigned)ppu->_ppuMask.intensifyBlue );

	// PPU Status
	_ui.checkPpuStatus_SpriteOverflow->setChecked( (unsigned)ppu->_ppuStatus.spriteOverflow );
	_ui.checkPpuStatus_Sprite0Hit->setChecked( (unsigned)ppu->_ppuStatus.sprite0Hit );
	_ui.checkPpuStatus_VBlankStarted->setChecked( (unsigned)ppu->_ppuStatus.vblankStarted );

	// Timing 
	_ui.lineTiming_PPUCycle->setText( QString::number(ppu->_cycleCountPerScanline) );
	_ui.lineTiming_Scanline->setText( QString::number(ppu->_currentScanline) );
	_ui.checkTiming_IsEvenFrame->setChecked( ppu->_isEvenFrame );

	// Current PPU address
	_ui.lineCurrent_CoarseXScroll->setText( QString::number((unsigned)ppu->_currentPpuAddress.coarseXScroll, 16).toUpper() );
	_ui.lineCurrent_CoarseYScroll->setText( QString::number((unsigned)ppu->_currentPpuAddress.coarseYScroll, 16).toUpper() );
	_ui.lineCurrent_NametableSelect->setText( QString::number((unsigned)ppu->_currentPpuAddress.nametableSelect) );
	_ui.lineCurrent_FineYScroll->setText( QString::number((unsigned)ppu->_currentPpuAddress.fineYScroll) );
	_ui.lineCurrent_FineXScroll->setText( QString::number(ppu->_fineXScroll) );
	_ui.checkCurrent_FirstWrite->setChecked( ppu->_firstWrite );

	// Temporary PPU address
	_ui.lineTemporary_CoarseXScroll->setText( QString::number((unsigned)ppu->_temporaryPpuAddress.coarseXScroll, 16).toUpper() );
	_ui.lineTemporary_CoarseYScroll->setText( QString::number((unsigned)ppu->_temporaryPpuAddress.coarseYScroll, 16).toUpper() );
	_ui.lineTemporary_NametableSelect->setText( QString::number((unsigned)ppu->_temporaryPpuAddress.nametableSelect) );
	_ui.lineTemporary_FineYScroll->setText( QString::number((unsigned)ppu->_temporaryPpuAddress.fineYScroll) );
}