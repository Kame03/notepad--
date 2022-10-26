#include "qtlangset.h"
#include "scintillaeditview.h"
#include "jsondeploy.h"
#include <qscilexer.h>
#include <QSettings>
#include <QColorDialog> 
#include <QMessageBox>
#include <QSpinBox>
#include <qDebug>

#if 0
//Ҫע����common.h�ı���һ�¡�
enum LangType {
	L_TEXT = -1, L_PHP, L_C, L_CPP, L_CS, L_OBJC, L_JAVA, L_RC, \
	L_HTML, L_XML, L_MAKEFILE, L_PASCAL, L_BATCH, L_INI, L_ASCII, L_USER, \
	L_ASP, L_SQL, L_VB, L_JS, L_CSS, L_PERL, L_PYTHON, L_LUA, \
	L_TEX, L_FORTRAN, L_BASH, L_FLASH, L_NSIS, L_TCL, L_LISP, L_SCHEME, \
	L_ASM, L_DIFF, L_PROPS, L_PS, L_RUBY, L_SMALLTALK, L_VHDL, L_KIX, L_AU3, \
	L_CAML, L_ADA, L_VERILOG, L_MATLAB, L_HASKELL, L_INNO, L_SEARCHRESULT, \
	L_CMAKE, L_YAML, L_COBOL, L_GUI4CLI, L_D, L_POWERSHELL, L_R, L_JSP, \
	L_COFFEESCRIPT, L_JSON, L_JAVASCRIPT, L_FORTRAN_77, L_BAANC, L_SREC, \
	L_IHEX, L_TEHEX, L_SWIFT, \
	L_ASN1, L_AVS, L_BLITZBASIC, L_PUREBASIC, L_FREEBASIC, \
	L_CSOUND, L_ERLANG, L_ESCRIPT, L_FORTH, L_LATEX, \
	L_MMIXAL, L_NIM, L_NNCRONTAB, L_OSCRIPT, L_REBOL, \
	L_REGISTRY, L_RUST, L_SPICE, L_TXT2TAGS, L_VISUALPROLOG, L_TYPESCRIPT, \
	L_EDIFACT, L_MARKDOWN, L_OCTAVE, L_PO, L_POV, L_IDL, L_GO, L_TXT, \
	// Don't use L_JS, use L_JAVASCRIPT instead
	// The end of enumated language type, so it should be always at the end
	L_EXTERNAL = 100,
};
#endif


QtLangSet::QtLangSet(QString initTag, QWidget *parent)
	: QMainWindow(parent), m_selectLexer(nullptr), m_selectStyleId(0), m_isStyleChange(false),m_isStyleChildChange(false), m_initShowLexerTag(initTag), m_first(true)
{
	ui.setupUi(this);

	initLangList();

	startSignSlot();
}

QtLangSet::~QtLangSet()
{
	if (nullptr != m_selectLexer)
	{
		delete m_selectLexer;
		m_selectLexer = nullptr;
	}
}

//�����Ƿ��ڹ��캯���У�����show��ᵼ�³�ʼ�ź��ҡ��ɴ��������show��Ϻ��ٿ����źţ������ʼ��
void QtLangSet::startSignSlot()
{
	connect(ui.langListWidget, &QListWidget::currentItemChanged, this, &QtLangSet::slot_itemSelect);
	connect(ui.styleListWidget, &QListWidget::currentItemChanged, this, &QtLangSet::slot_styleItemSelect);

	connect(ui.boldCheckBox, &QCheckBox::stateChanged, this, &QtLangSet::slot_fontBoldChange);
	connect(ui.italicCheckBox, &QCheckBox::stateChanged, this, &QtLangSet::slot_fontItalicChange);
	connect(ui.underlineCheckBox, &QCheckBox::stateChanged, this, &QtLangSet::slot_fontUnderlineChange);
	connect(ui.fontSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slot_fontSizeChange(int)));
	connect(ui.fontComboBox, &QFontComboBox::currentFontChanged, this, &QtLangSet::slot_fontChange);
}

void QtLangSet::slot_fontBoldChange(int state)
{
	//��ʱ���÷��
	if (m_selectLexer != nullptr)
	{
		if (m_curStyleData.font.bold() != ((state == Qt::Unchecked) ? false : true))
		{
			m_curStyleData.font.setBold((state == Qt::Unchecked) ? false : true);
			m_selectLexer->setFont(m_curStyleData.font, m_selectStyleId);
			m_isStyleChange = true;
			emit viewStyleChange(m_selectLexer->lexerId(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
		}
	}
}

void QtLangSet::slot_fontItalicChange(int state)
{
	//��ʱ���÷��
	if (m_selectLexer != nullptr)
	{
		if (m_curStyleData.font.italic() != ((state == Qt::Unchecked) ? false : true))
		{
			m_curStyleData.font.setItalic((state == Qt::Unchecked) ? false : true);
			m_selectLexer->setFont(m_curStyleData.font, m_selectStyleId);
			m_isStyleChange = true;
			emit viewStyleChange(m_selectLexer->lexerId(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
		}
	}
}

void QtLangSet::slot_fontUnderlineChange(int state)
{
	//��ʱ���÷��
	if (m_selectLexer != nullptr)
	{
		if (m_curStyleData.font.underline() != ((state == Qt::Unchecked) ? false : true))
		{
			m_curStyleData.font.setUnderline((state == Qt::Unchecked) ? false : true);
			m_selectLexer->setFont(m_curStyleData.font, m_selectStyleId);
			m_isStyleChange = true;
			emit viewStyleChange(m_selectLexer->lexerId(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
		}
	}
}

void QtLangSet::slot_fontSizeChange(int v)
{
	//��ʱ���÷��
	if (m_selectLexer != nullptr)
	{
		if (m_curStyleData.font.pointSize() != v)
		{
			m_curStyleData.font.setPointSize(v);
			m_selectLexer->setFont(m_curStyleData.font, m_selectStyleId);
			m_isStyleChange = true;
			emit viewStyleChange(m_selectLexer->lexerId(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
		}
	}
}

void QtLangSet::slot_fontChange(const QFont &font)
{
	//��ʱ���÷��
	if (m_selectLexer != nullptr)
	{
		if (m_curStyleData.font != font)
		{
			QFont oldf = m_curStyleData.font;
			m_curStyleData.font = font;
			m_curStyleData.font.setBold(oldf.bold());
			m_curStyleData.font.setItalic(oldf.italic());
			m_curStyleData.font.setUnderline(oldf.underline());

			m_selectLexer->setFont(m_curStyleData.font, m_selectStyleId);
			m_isStyleChange = true;
			emit viewStyleChange(m_selectLexer->lexerId(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
		}
	}
}

void QtLangSet::initLangList()
{

	for (int i = 0; i <= L_TXT; ++i)
	{
		QsciLexer *pLexer = ScintillaEditView::createLexer(i);
		if (nullptr != pLexer)
		{
			QString langName = pLexer->lexerTag();
			//qDebug() << langName << pLexer->lexer();
			QListWidgetItem *item = new QListWidgetItem(langName);
			item->setData(Qt::UserRole,i);
			ui.langListWidget->addItem(item);
			delete pLexer;
		}
		
	}

	//��������Ƕ������ӵģ����ӳ�ִ�С������ǵ���ѡ���ѡ���У����ǵ�һ��asp��
	ui.langListWidget->sortItems(Qt::AscendingOrder);

}

//�����ǰ������item
void QtLangSet::slot_itemSelect(QListWidgetItem *item, QListWidgetItem *previous)
{
	//��һ�α����Գ�ʼ��lexid��ʾ��������ʵ������Ϊ��ʼ��list�����󣬻��Զ��ѵ�һ������Ϊ��ǰ��
	//��ª��ʹ���滻��ʵ�֡��ô�ʱ��س��ֵ�ǰedit���﷨����
	if (m_first && !m_initShowLexerTag.isEmpty())
	{
		m_first = false;

		QList<QListWidgetItem*> destItems = ui.langListWidget->findItems(m_initShowLexerTag,Qt::MatchExactly);
		if (!destItems.isEmpty())
		{
			item = destItems.first();
			ui.langListWidget->setCurrentItem(item);
		}
	}
	//����ǰһ���Ѿ��޸ĵġ�ѯ���û��Ƿ񱣴�
	if (m_isStyleChange && previous != nullptr && m_selectLexer !=nullptr)
	{
		if (QMessageBox::Yes == QMessageBox::question(this, tr("Save Change"), tr("%1 style configuration has been modified. Do you want to save it?").arg(previous->text())))
		{
			saveCurLangSettings();
		}
	}

	if (item == nullptr)
	{
		return;
	}

	if (!item->isSelected())
	{
		item->setSelected(true);
	}

	int lexId = item->data(Qt::UserRole).toInt();

	ui.styleListWidget->clear();

	if (m_selectLexer != nullptr)
	{
		delete m_selectLexer;
		m_selectLexer = nullptr;
	}

	QsciLexer *pLexer = ScintillaEditView::createLexer(lexId);
	if (nullptr != pLexer)
	{
		m_selectLexer = pLexer;

		for (int i = 0; i <= 255; ++i)
		{
			QString desc = pLexer->description(i);
			if (!desc.isEmpty())
			{
				QListWidgetItem *itemtemp = new QListWidgetItem(desc);
				itemtemp->setData(Qt::UserRole, i);
				ui.styleListWidget->addItem(itemtemp);
			}
		}
	}

	//�Զ���ʾ��һ���Ľ��
	QListWidgetItem *itemtemp = ui.styleListWidget->item(0);
	if (itemtemp != nullptr)
	{
		itemtemp->setSelected(true);
		slot_styleItemSelect(itemtemp, nullptr);
}

	m_isStyleChange = false;
}

//��ȡ��ǰui�ϵ�����
void QtLangSet::getCurUiFont(QFont& font)
{
	font = ui.fontComboBox->currentFont();
	font.setBold(ui.boldCheckBox->isChecked());
	font.setItalic(ui.italicCheckBox->isChecked());
	font.setUnderline(ui.underlineCheckBox->isChecked());
	font.setPointSize(ui.fontSpinBox->value());
}

void QtLangSet::closeEvent(QCloseEvent * e)
{
	saveCurLangSettings();
}

//�����ǰ�ķ��item
void QtLangSet::slot_styleItemSelect(QListWidgetItem *current, QListWidgetItem *previous)
	{
	if (current != nullptr && m_selectLexer != nullptr)
	{
		int styleId = current->data(Qt::UserRole).toInt();
		m_selectStyleId = styleId;
		QsciLexer::StyleData & sd = m_selectLexer->styleData(styleId);
		m_curStyleData = sd;
		setStyleShow(sd.font, sd.color, sd.paper);
		m_isStyleChange = false;
	}
}

void QtLangSet::fillForegroundColor(QColor& fcColor)
{
	QPixmap f(32, 32);
	f.fill(fcColor);
	ui.label_fc->setPixmap(f);
}

void QtLangSet::fillBackgroundColor(QColor &bkColor)
{

	QPixmap b(32, 32);
	b.fill(bkColor);
	ui.label_bc->setPixmap(b);
}

void QtLangSet::setStyleShow(QFont& font, QColor& fcColor, QColor &bkColor)
{
	ui.fontComboBox->setCurrentFont(font);
	ui.fontSpinBox->setValue(font.pointSize());

	ui.boldCheckBox->setChecked(font.bold());
	ui.italicCheckBox->setChecked(font.italic());
	ui.underlineCheckBox->setChecked(font.underline());

	fillForegroundColor(fcColor);
	fillBackgroundColor(bkColor);
	
}

//��ȡ�ض����Ե�����
bool QtLangSet::readLangSettings(QsciLexer *lexer, QString tag)
{
	QString cfgPath = QString("notepad/%1").arg(tag);

	QSettings qs(QSettings::IniFormat, QSettings::UserScope, cfgPath);
	return lexer->readSettings(qs);
}

#if 0
void QtLangSet::setCurSelectLang(QString tag)
{
	QList<QListWidgetItem*> destItem = ui.langListWidget->findItems(tag, Qt::MatchExactly);
	if (!destItem.isEmpty())
	{
		QListWidgetItem* itemtemp = destItem.first();
		itemtemp->setSelected(true);
		//slot_itemSelect(itemtemp, nullptr);
	}
}
#endif

bool QtLangSet::saveCurLangSettings()
{
	if (m_isStyleChange && m_selectLexer != nullptr)
	{
		QString tag = m_selectLexer->lexerTag();
		QString cfgPath = QString("notepad/%1").arg(tag);

		QSettings qs(QSettings::IniFormat, QSettings::UserScope, cfgPath);
		m_selectLexer->writeSettings(qs);
		m_isStyleChange = false;
		return true;
	}
	return false;
}

//�޸�ǰ��ɫ
void QtLangSet::slot_changeFgColor()
{
	QColor color = QColorDialog::getColor(Qt::white, this, tr("Style Foreground Color"));
	if (color.isValid())
	{
		//��ǰǰ��ɫ�Ƿ�仯
		if (m_curStyleData.color != color)
		{
			m_curStyleData.color = color;
			fillForegroundColor(color);
			m_isStyleChange = true;

			//��ʱ���÷��
			if (m_selectLexer != nullptr)
			{
				m_selectLexer->setColor(color,m_selectStyleId);

				emit viewStyleChange(m_selectLexer->lexerId(), m_selectStyleId, color, m_curStyleData.paper, m_curStyleData.font,false);
			}
		}
	}
	
}

//�޸ı���ɫ
void QtLangSet::slot_changeBkColor()
{
	QColor color = QColorDialog::getColor(Qt::white, this, tr("Style Background Color"));
	if (color.isValid())
	{
		//��ǰǰ��ɫ�Ƿ�仯
		if (m_curStyleData.paper != color)
		{
			m_curStyleData.paper = color;
			fillBackgroundColor(color);
			m_isStyleChange = true;

			//��ʱ���÷��
			if (m_selectLexer != nullptr)
			{
				m_selectLexer->setPaper(color, m_selectStyleId);
				emit viewStyleChange(m_selectLexer->lexerId(), m_selectStyleId, color, m_curStyleData.paper, m_curStyleData.font, false);
			}
		}
	}
}

void QtLangSet::slot_saveClick()
{
	saveCurLangSettings();
}
