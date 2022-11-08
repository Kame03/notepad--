#include "qtlangset.h"
#include "scintillaeditview.h"
#include "jsondeploy.h"
#include "rcglobal.h"
#include <SciLexer.h>
#include <qscilexer.h>
#include <QSettings>
#include <QColorDialog> 
#include <QMessageBox>
#include <QSpinBox>
#include <QDebug>
#include <QDir>

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
	L_EXTERNAL = 100, L_USER_DEFINE = 200
};
#endif


QtLangSet::QtLangSet(QString initTag, QWidget *parent)
	: QMainWindow(parent), m_selectLexer(nullptr), m_selectStyleId(0), m_isStyleChange(false),m_isStyleChildChange(false), m_initShowLexerTag(initTag), m_previousSysLangItem(nullptr)
{
	ui.setupUi(this);
	initLangList();
	initUserDefineLangList();

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



void QtLangSet::startSignSlot()
{
	//��ʼ����ʹ�õ���QListWidget::currentItemChanged�źţ����Ƿ��ָ��źŴ���Ī������Ĵ������ơ�
	//������QT5.12��bug�����Ի���itemClicked�ź�
	connect(ui.langListWidget, &QListWidget::itemClicked, this, &QtLangSet::slot_itemSelect);
	connect(ui.userLangListWidget, &QListWidget::itemClicked, this, &QtLangSet::slot_userLangItemSelect);


	connect(ui.styleListWidget, &QListWidget::currentItemChanged, this, &QtLangSet::slot_styleItemSelect);

	connect(ui.boldCheckBox, &QCheckBox::stateChanged, this, &QtLangSet::slot_fontBoldChange);
	connect(ui.italicCheckBox, &QCheckBox::stateChanged, this, &QtLangSet::slot_fontItalicChange);
	connect(ui.underlineCheckBox, &QCheckBox::stateChanged, this, &QtLangSet::slot_fontUnderlineChange);
	connect(ui.fontSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slot_fontSizeChange(int)));
	connect(ui.fontComboBox, &QFontComboBox::currentFontChanged, this, &QtLangSet::slot_fontChange);
}

//�ó�ʼ��ʱѡ��ǰ�༭��������
void QtLangSet::selectInitLangTag(QString initLangTag)
{

	//��һ�α����Գ�ʼ��lexid��ʾ��������ʵ������Ϊ��ʼ��list�����󣬻��Զ��ѵ�һ������Ϊ��ǰ��
	//���ﻹ�������⣬��qt5.12.4��bug���Ƶ���:���û��ѡ��QListWidget���������Զ�ѡ���һ��
	if (!initLangTag.isEmpty())
	{
		QList<QListWidgetItem*> destItems = ui.langListWidget->findItems(initLangTag,Qt::MatchExactly);
		if (!destItems.isEmpty())
		{
			QListWidgetItem* newItem = destItems.first();

			ui.langListWidget->scrollToItem(newItem);
			ui.langListWidget->setCurrentItem(newItem);
			ui.userLangListWidget->setCurrentItem(nullptr);

			slot_itemSelect(newItem);
		}
		else
		{
			destItems = ui.userLangListWidget->findItems(initLangTag, Qt::MatchExactly);
			if (!destItems.isEmpty())
			{
				QListWidgetItem* userItem = destItems.first();
				ui.userLangListWidget->scrollToItem(userItem);
				ui.userLangListWidget->setCurrentItem(userItem);
				ui.langListWidget->setCurrentItem(nullptr);

				slot_userLangItemSelect(userItem);

			}
		}
	}
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
			emit viewStyleChange(m_selectLexer->lexerTag(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
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
			emit viewStyleChange(m_selectLexer->lexerTag(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
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
			emit viewStyleChange(m_selectLexer->lexerTag(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
		}
	}
}

void QtLangSet::slot_fontSizeChange(int v)
{
	//��ʱ���÷��
	if (m_selectLexer != nullptr)
	{
		if (!ui.modifyAllFont->isChecked())
		{
		if (m_curStyleData.font.pointSize() != v)
		{
			m_curStyleData.font.setPointSize(v);
			m_selectLexer->setFont(m_curStyleData.font, m_selectStyleId);
			m_isStyleChange = true;
				emit viewStyleChange(m_selectLexer->lexerTag(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
		}
	}
		else
		{
			m_curStyleData.font.setPointSize(v);
			m_selectLexer->setFont(m_curStyleData.font, m_selectStyleId);
			m_isStyleChange = true;
			//��ʱ���÷��
			if (m_selectLexer != nullptr)
			{
				int row = ui.styleListWidget->count();

				QListWidgetItem* item = nullptr;

				for (int i = 0; i < row; ++i)
				{
					item = ui.styleListWidget->item(i);
					int styleId = item->data(Qt::UserRole).toInt();

					//���������С�޸�Ϊһ��
					QFont f = m_selectLexer->font(styleId);
					f.setPointSize(v);
					m_selectLexer->setFont(f, styleId);
}

				saveCurLangSettings();
				emit viewLexerChange(m_selectLexer->lexerTag());
			}
		}
	}
}

void QtLangSet::slot_fontChange(const QFont &font)
{
	//��ʱ���÷��
	if (m_selectLexer != nullptr)
	{
		if (!ui.modifyAllFont->isChecked())
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
				emit viewStyleChange(m_selectLexer->lexerTag(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
		}
	}
		else
		{
			//ͳһ�޸����壬��С�ͷ�񱣴治��
			QFont oldf = m_curStyleData.font;
			m_curStyleData.font = font;
			m_curStyleData.font.setBold(oldf.bold());
			m_curStyleData.font.setItalic(oldf.italic());
			m_curStyleData.font.setUnderline(oldf.underline());

			m_isStyleChange = true;
			//��ʱ���÷��
			if (m_selectLexer != nullptr)
			{
				int row = ui.styleListWidget->count();

				QListWidgetItem* item = nullptr;

				for (int i = 0; i < row; ++i)
				{
					item = ui.styleListWidget->item(i);
					int styleId = item->data(Qt::UserRole).toInt();
					QFont f = m_selectLexer->font(styleId);

					//���������ͳһ���ã����Ƿ�������С�����ϵ�һ��
					QFont newFont = m_curStyleData.font;
					newFont.setBold(f.bold());
					newFont.setItalic(f.italic());
					newFont.setUnderline(f.underline());
					newFont.setPointSize(f.pointSize());

					m_selectLexer->setFont(newFont, styleId);
}

				saveCurLangSettings();
				emit viewLexerChange(m_selectLexer->lexerTag());
			}
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

	//��������Ƕ������ӵģ����ӳ�ִ�С������ǵ���ѡ���ѡ���У����ǵ�һ������asp��
	ui.langListWidget->sortItems(Qt::AscendingOrder);

}

//isLoadToUI�Ƿ������ʾ����ǰUI����
bool QtLangSet::readLangSetFile(QString langName,QString &keyword, QString &motherLang, QString & extTypes)
{
	QString userLangFile = QString("notepad/userlang/%1").arg(langName);//�Զ��������в�����.�ַ�����������д�����Ҫ���
	QSettings qs(QSettings::IniFormat, QSettings::UserScope, userLangFile);
	qs.setIniCodec("UTF-8");
	qDebug() << qs.fileName();

	if (!qs.contains("mz"))
	{
		return false;
	}

	//�Զ������Ը�ʽ��
	//mz:ndd
	//name:xxx
	//mother:xxx none/cpp/html ������
	//ext:xx xx xx �ļ�������׺��
	//keword:xxx
	keyword = qs.value("keyword").toString();
	extTypes = qs.value("ext").toString();
	motherLang = qs.value("mother").toString();

	return true;
}

//��ʼ���û��Զ������������
void QtLangSet::initUserDefineLangList()
		{
	QString userLangDirPath = getUserLangDirPath();

	//�����ļ���
	QDir dir_file(userLangDirPath);
	QFileInfoList list_file = dir_file.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name);

	for (int i = 0; i < list_file.size(); ++i)
	{  //����ǰĿ¼�������ļ���ӵ�treewidget��
		QFileInfo fileInfo = list_file.at(i);

		//����ļ���ext��tag��ӳ���ļ����������ý���
		if (fileInfo.baseName() == "ext_tag")
		{
			continue;
		}

		ui.userLangListWidget->addItem(fileInfo.baseName());
	}
	ui.userLangListWidget->sortItems(Qt::AscendingOrder);
}


//�л���ǰ���ԡ�
//QListWidget::currentItemChanged����ź�qt5.12.10��Ӧ���Ǵ���bug,����Ī�������Ҫ��һ��currentItemChanged�źų��������µ�һ�θ�ѡ������ǡ�
//����itemClicked�ź�
void QtLangSet::slot_itemSelect(QListWidgetItem *item)
{
	if (item == nullptr)
	{
		return;
		}
	//qDebug() << item->text() << (previous ? previous->text() : 0);


	ui.langListWidget->scrollToItem(item);

	ui.userLangListWidget->setCurrentItem(nullptr);

	//����ǰһ���Ѿ��޸ĵġ�ѯ���û��Ƿ񱣴�
	if (m_isStyleChange && m_previousSysLangItem != nullptr && m_selectLexer !=nullptr)
	{
		if (QMessageBox::Yes == QMessageBox::question(this, tr("Save Change"), tr("%1 style configuration has been modified. Do you want to save it?").arg(m_previousSysLangItem->text())))
		{
			saveCurLangSettings();
		}
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

		QString keyword(m_selectLexer->keywords(1));

		ui.keywordTe->setPlainText(keyword);
		ui.motherLangCb->setCurrentIndex(0);

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

	m_previousSysLangItem = item;

	//�Զ���ʾ��һ���Ľ��
	QListWidgetItem *itemtemp = ui.styleListWidget->item(0);
	if (itemtemp != nullptr)
	{
		itemtemp->setSelected(true);
		slot_styleItemSelect(itemtemp, nullptr);
}

	m_isStyleChange = false;

	ui.extFileType->clear();
	ui.motherLangCb->setCurrentIndex(0);
}

//����ĸ��ģ�����ԣ������Է����ʾ����
void QtLangSet::displayUserMotherLangsStyle(QString langTagName, UserLangMother motherLang)
{
	QsciLexer *pLexer = nullptr;
	
	ui.styleListWidget->clear();

	switch (motherLang)
	{
	case MOTHER_NONE:
		pLexer = ScintillaEditView::createLexer(L_USER_TXT, langTagName);
		break;
	case MOTHER_CPP:
		pLexer = ScintillaEditView::createLexer(L_USER_CPP, langTagName);
		break;
	default:
		pLexer = ScintillaEditView::createLexer(L_USER_TXT, langTagName);
		break;
	}

	if (nullptr != pLexer)
	{
		//����һ��Ҫ�������Ե�������Ϊlexer��tag��
		//pLexer->setLexerTag(langTagName);
		pLexer->setIsUserDefineKeywords(true);

		if (m_selectLexer != nullptr)
		{
			delete m_selectLexer;
		}

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


//�����ǰ�û��Զ������Ե�����item��todo:��ʱû����ǰһ�����޸ı��棬��Ҫ�û�ÿ���޸ĺ��Լ����save��������ܲ��ᱣ��
void QtLangSet::slot_userLangItemSelect(QListWidgetItem *item)
{
	if (item == nullptr)
	{
		return;
	}

	m_previousSysLangItem = nullptr;
	ui.langListWidget->setCurrentItem(nullptr);

	QString langName = item->text();

	QString keyword;
	QString motherLang;
	QString extTypes;

	if (readLangSetFile(langName, keyword, motherLang, extTypes))
	{
		ui.keywordTe->setPlainText(keyword);
		ui.motherLangCb->setCurrentText(motherLang);
		ui.extFileType->setText(extTypes);

		//����ĸ��ģ�����ԣ������Է����ʾ����
		displayUserMotherLangsStyle(langName, (UserLangMother)(LangType::L_USER_TXT + ui.motherLangCb->currentIndex()));
	}
	else
	{
		ui.statusBar->showMessage(tr("Read %1 language user define  format error.").arg(langName), 10000);
	}
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

//��item�Ķ�Ӧ�����ʾ�ڽ�����
void QtLangSet::syncShowStyleItemToUI(QListWidgetItem *item)
{
	if (item != nullptr && m_selectLexer != nullptr)
	{
		int styleId = item->data(Qt::UserRole).toInt();
		m_selectStyleId = styleId;
		QsciLexer::StyleData & sd = m_selectLexer->styleData(styleId);
		m_curStyleData = sd;
		setStyleShow(sd.font, sd.color, sd.paper);
		m_isStyleChange = false;
	}
}
//�����ǰ�ķ��item
void QtLangSet::slot_styleItemSelect(QListWidgetItem *current, QListWidgetItem *previous)
	{
#if 0
	if (current != nullptr && m_selectLexer != nullptr)
	{
		int styleId = current->data(Qt::UserRole).toInt();
		m_selectStyleId = styleId;
		QsciLexer::StyleData & sd = m_selectLexer->styleData(styleId);
		m_curStyleData = sd;
		setStyleShow(sd.font, sd.color, sd.paper);
		m_isStyleChange = false;
	}
#endif
	syncShowStyleItemToUI(current);
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
	if (QFile::exists(qs.fileName()))
	{
	return lexer->readSettings(qs);
}
	return false;
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
		if (!ui.modiryAllColor->isChecked())
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
					m_selectLexer->setColor(color, m_selectStyleId);

					emit viewStyleChange(m_selectLexer->lexerTag(), m_selectStyleId, color, m_curStyleData.paper, m_curStyleData.font, false);
			}
		}
	}
		else
		{
			//ȫ�������ɫ�޸�
			//��ǰǰ��ɫ�Ƿ�仯
			m_curStyleData.color = color;
			fillForegroundColor(color);
			m_isStyleChange = true;

			//��ʱ���÷��
			if (m_selectLexer != nullptr)
			{
				int row = ui.styleListWidget->count();

				QListWidgetItem* item = nullptr;

				for (int i = 0; i < row; ++i)
				{
					item = ui.styleListWidget->item(i);
					int styleId = item->data(Qt::UserRole).toInt();
					m_selectLexer->setColor(color, styleId);
				}

				saveCurLangSettings();
				emit viewLexerChange(m_selectLexer->lexerTag());
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
				emit viewStyleChange(m_selectLexer->lexerTag(), m_selectStyleId, color, m_curStyleData.paper, m_curStyleData.font, false);
			}
		}
	}
}

void QtLangSet::slot_saveClick()
{
	saveCurLangSettings();
}



//�ָ���ʼ������
void QtLangSet::slot_reset()
{
	if (m_selectLexer != nullptr)
	{
		if (QMessageBox::Yes != QMessageBox::question(this, tr("Reset Style"), tr("Are you sure to reset language %1 sytle").arg(m_selectLexer->lexerTag())))
		{
			return;
		}
		//��������Զ�������ã�Ҳɾ����
		QString cfgPath = QString("notepad/%1").arg(m_selectLexer->lexerTag());

		QSettings qs(QSettings::IniFormat, QSettings::UserScope, cfgPath);
		if (QFile::exists(qs.fileName()))
		{
			QFile::remove(qs.fileName());
		}

		//һ��Ҫ�ȱ��棬��ΪselectInitLangTag����ܻ�仯

		int row = ui.styleListWidget->currentRow();

		selectInitLangTag(m_selectLexer->lexerTag());

		//�ֶ�ˢ��UI
		QListWidgetItem* styleItem = ui.styleListWidget->item(row);

		syncShowStyleItemToUI(styleItem);

		emit viewLexerChange(m_selectLexer->lexerTag());
	}
}
