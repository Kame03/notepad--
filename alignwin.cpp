#include "alignwin.h"

AlignWin::AlignWin(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

AlignWin::~AlignWin()
{}

void AlignWin::slot_ok()
{
    bool ok = true;
    bool ok1 = true;
	int leftStart = ui.leftStart->text().toInt(&ok) - 1;
	int leftEnd = ui.leftEnd->text().toInt(&ok1) - 1;

    if(!ok || !ok1)
    {
        return;
    }

    if((leftStart < 0) || (leftStart > leftEnd))
    {
        return;
    }

	int type = 0;

	//0��򵥵ģ����ߴ��ֱ𵥶���ʾ���Է�һ����������չʾ
	emit alignLine(type, leftStart, leftEnd);

	close();
}
