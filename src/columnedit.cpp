#include "columnedit.h"
#include "scintillaeditview.h"

ColumnEdit::ColumnEdit(QWidget *parent)
	: QWidget(parent), m_curEditWin(nullptr), m_editTabWidget(nullptr)
{
	ui.setupUi(this);

	connect(ui.addPrefix, &QCheckBox::stateChanged, this, &ColumnEdit::slot_addPrefix);
}

ColumnEdit::~ColumnEdit()
{
}


void ColumnEdit::slot_insertTextEnable(bool check)
{
	if (check)
	{
		if (ui.numGroupBox->isChecked())
		{
			ui.numGroupBox->setChecked(!check);
		}
	}
}

void ColumnEdit::slot_insertNumEnable(bool check)
{
	if (check)
	{
		if (ui.textGroupBox->isChecked())
		{
			ui.textGroupBox->setChecked(!check);
		}
	}
}

void ColumnEdit::slot_addPrefix(int s)
{
	if (s == Qt::Checked)
	{
		ui.prefix->setEnabled(true);
	}
	else
	{
		ui.prefix->setEnabled(false);
	}
}

//�Զ�������ǰ���ڵ�״̬
QWidget* ColumnEdit::autoAdjustCurrentEditWin()
{
	QWidget* pw = m_editTabWidget->currentWidget();

	if (m_curEditWin != pw)
	{
		m_curEditWin = pw;
	}
	return pw;
}

void ColumnEdit::setTabWidget(QTabWidget *editTabWidget)
{
	m_editTabWidget = editTabWidget;
}

void ColumnEdit::slot_ok()
{
	autoAdjustCurrentEditWin();

	ScintillaEditView* pEdit = dynamic_cast<ScintillaEditView*>(m_curEditWin);
	if (pEdit == nullptr || pEdit->isReadOnly())
	{
		QApplication::beep();
		return;
	}

	QString text;

	bool isNum = false;
	int numType = 10;
	int initNum = 0;
	int inc = ui.incNum->value();
	int repeNum = ui.repeNum->value();
	bool addPrefix = ui.addPrefix->isChecked();
	QString prefix = ui.prefix->text();

	//�ǲ����ı�ģʽ
	if (ui.textGroupBox->isChecked())
	{

		text = ui.inputText->text();
	}
	else
	{
		isNum = true;

		text = ui.initNum->text();

		bool ok = false;

		if (ui.is10->isChecked())
		{
			numType = 10;
		}
		else if (ui.is16->isChecked())
		{
			numType = 16;
		}
		else if (ui.is8->isChecked())
		{
			numType = 8;
		}
		else if (ui.is2->isChecked())
		{
			numType = 2;
		}
		int num = text.toInt(&ok, 10);

		initNum = num;

		if (ok)
		{
			text = QString::number(num, numType);

			if (addPrefix)
			{
				text = prefix + text;
			}
		}
		else
		{
			QApplication::beep();
			return;
		}
	}

	pEdit->execute(SCI_BEGINUNDOACTION);

	auto cursorPos = pEdit->execute(SCI_GETCURRENTPOS);
	auto cursorCol = pEdit->execute(SCI_GETCOLUMN, cursorPos);
	auto cursorLine = pEdit->execute(SCI_LINEFROMPOSITION, cursorPos);
	auto endPos = pEdit->execute(SCI_GETLENGTH);
	auto endLine = pEdit->execute(SCI_LINEFROMPOSITION, endPos);
	
	QByteArray lineData;

	int rn = repeNum;

	for (size_t i = cursorLine; i <= static_cast<size_t>(endLine); ++i)
	{
		auto lineBegin = pEdit->execute(SCI_POSITIONFROMLINE, i);
		auto lineEnd = pEdit->execute(SCI_GETLINEENDPOSITION, i);

		auto lineEndCol = pEdit->execute(SCI_GETCOLUMN, lineEnd);
		auto lineLen = lineEnd - lineBegin;
		lineData.resize(lineLen);
			

		Sci_TextRange  lineText;
		lineText.chrg.cpMin = static_cast<Sci_Position>(lineBegin);
		lineText.chrg.cpMax = static_cast<Sci_Position>(lineEnd);
		lineText.lpstrText = lineData.data();
		//��ȡԭʼ�е�����
		pEdit->SendScintilla(SCI_GETTEXTRANGE, 0, &lineText);

		if (lineEndCol < cursorCol)
		{
			QByteArray s_space(cursorCol - lineEndCol, ' ');
			lineData.append(s_space);
			lineData.append(text);
		}
		else
		{
			int posAbs2Start = pEdit->execute(SCI_FINDCOLUMN, i, cursorCol);
			int posRelative2Start = posAbs2Start - lineBegin;
			lineData.insert(posRelative2Start, text);
		}

		pEdit->SendScintilla(SCI_SETTARGETRANGE, lineBegin, lineEnd);
		pEdit->SendScintilla(SCI_REPLACETARGET, lineData.size(), lineData.data());

		if (isNum)
		{
			--rn;

			if (rn > 0)
			{

			}
			else
			{
				rn = repeNum;
				initNum += inc;

			}
			
			text = QString::number(initNum, numType);


			if (addPrefix)
			{
				text = prefix + text;
			}
		}
	}

	pEdit->execute(SCI_ENDUNDOACTION);
}