#pragma once

#include <QDialog>
#include "ui_ouputViewer.h"
#pragma execution_character_set("utf-8")
class ouputViewer : public QDialog
{
	Q_OBJECT
public:
	~ouputViewer();
	static ouputViewer* GetOPViewerInstance();
	void loopDMXstatus();

private:
	Ui::ouputViewer ui;
	ouputViewer(QWidget *parent = Q_NULLPTR);
	};
