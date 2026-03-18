#ifndef EDITIDDIALOG_H
#define EDITIDDIALOG_H

#include <QDialog>

namespace Ui {
class EditIDDialog;
}

class EditIDDialog : public QDialog
{
	Q_OBJECT

public:
	explicit EditIDDialog(QWidget *parent = 0);
	~EditIDDialog();

	void setData(int old_id);

	int m_iNewID;

private slots:
	void on_buttonBox_accepted();

private:
	Ui::EditIDDialog *ui;
};

#endif // EDITIDDIALOG_H
