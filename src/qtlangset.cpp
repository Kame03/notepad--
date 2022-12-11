#include "qtlangset.h"
#include "scintillaeditview.h"
#include "nddsetting.h"
#include "rcglobal.h"
#include "ccnotepad.h"
#include "styleset.h"

#include <SciLexer.h>
#include <qscilexer.h>
#include <QSettings>
#include <QColorDialog> 
#include <QMessageBox>
#include <QSpinBox>
//#include <QDebug>
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

static const QColor blackColor(Qt::black);
static const QColor lightColor(0xdedede);

static const QColor blackColor1(0x0000ff);
static const QColor lightColor1(0xffaa00);

//C++ע�͵�Ĭ����ɫ��������
static const QColor blackColor2(0x007f00);
static const QColor lightColor2(0x009000);

static const QColor blackColor3(0x7f7f00);
static const QColor lightColor3(0xfca287);

static const QColor blackColor4(0x8000ff);
static const QColor lightColor4(0xffaa00);

static const QColor blackColor5(0x007f7f);
static const QColor lightColor5(0xaaff7f);

static const QColor blackColor6(0x7f007f);
static const QColor lightColor6(0x00ffff);

QMap<QString, QColor> QtLangSet::s_darkColorMap;

static void initDarkColorMap()
{
	if (QtLangSet::s_darkColorMap.isEmpty())
	{
		QtLangSet::s_darkColorMap.insert(blackColor.name(), lightColor);
		QtLangSet::s_darkColorMap.insert(blackColor1.name(), lightColor1);
		QtLangSet::s_darkColorMap.insert(blackColor2.name(), lightColor2);
		QtLangSet::s_darkColorMap.insert(blackColor3.name(), lightColor3);
		QtLangSet::s_darkColorMap.insert(blackColor4.name(), lightColor4);
		QtLangSet::s_darkColorMap.insert(blackColor5.name(), lightColor5);
		QtLangSet::s_darkColorMap.insert(blackColor6.name(), lightColor6);
	}
}

QtLangSet::QtLangSet(QString initTag, QWidget *parent)
	: QMainWindow(parent), m_selectLexer(nullptr), m_selectStyleId(0), m_isStyleChange(false),m_isStyleChildChange(false), m_initShowLexerTag(initTag), m_previousSysLangItem(nullptr),m_isGlobelItem(false)
{
	ui.setupUi(this);
	initLangList();
	initUserDefineLangList();

	startSignSlot();
	initDarkColorMap();
}

QtLangSet::~QtLangSet()
{
	if (nullptr != m_selectLexer)
	{
		delete m_selectLexer;
		m_selectLexer = nullptr;
	}
}


//��ͬ������ʱ������ȹرչ����ۺ���������ѭ��������������ʹ��
void QtLangSet::enableFontChangeSensitive(bool isSensitive)
{
	if (isSensitive)
	{
		connect(ui.boldCheckBox, &QCheckBox::stateChanged, this, &QtLangSet::slot_fontBoldChange);
		connect(ui.italicCheckBox, &QCheckBox::stateChanged, this, &QtLangSet::slot_fontItalicChange);
		connect(ui.underlineCheckBox, &QCheckBox::stateChanged, this, &QtLangSet::slot_fontUnderlineChange);
		connect(ui.fontSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slot_fontSizeChange(int)));
		connect(ui.fontComboBox, &QFontComboBox::currentFontChanged, this, &QtLangSet::slot_fontChange);
	}
	else
	{
		disconnect(ui.boldCheckBox, &QCheckBox::stateChanged, this, &QtLangSet::slot_fontBoldChange);
		disconnect(ui.italicCheckBox, &QCheckBox::stateChanged, this, &QtLangSet::slot_fontItalicChange);
		disconnect(ui.underlineCheckBox, &QCheckBox::stateChanged, this, &QtLangSet::slot_fontUnderlineChange);
		disconnect(ui.fontSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slot_fontSizeChange(int)));
		disconnect(ui.fontComboBox, &QFontComboBox::currentFontChanged, this, &QtLangSet::slot_fontChange);
	}
}

void QtLangSet::startSignSlot()
{
	//��ʼ����ʹ�õ���QListWidget::currentItemChanged�źţ����Ƿ��ָ��źŴ���Ī������Ĵ������ơ�
	//������QT5.12��bug�����Ի���itemClicked�ź�
	connect(ui.langListWidget, &QListWidget::itemClicked, this, &QtLangSet::slot_itemSelect);
	connect(ui.userLangListWidget, &QListWidget::itemClicked, this, &QtLangSet::slot_userLangItemSelect);
	connect(ui.styleListWidget, &QListWidget::itemClicked, this, &QtLangSet::slot_styleItemSelect);

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

			if (m_isGlobelItem)
			{
				if (ui.useGbFontBold->isChecked())
				{
					slot_useAlobalFontBold(true);
				}
			}
			else
			{
				emit viewStyleChange(m_selectLexer->lexerTag(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
			}
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
			if (m_isGlobelItem)
			{
				if (ui.useGbFontItalic->isChecked())
				{
					slot_useAlobalFontItalic(true);
				}
			}
			else
			{
				emit viewStyleChange(m_selectLexer->lexerTag(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
			}
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
			if (m_isGlobelItem)
			{
				if (ui.useGbFontUnderline->isChecked())
				{
					slot_useAlobalFontUnderline(true);
				}
			}
			else
			{
				emit viewStyleChange(m_selectLexer->lexerTag(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
			}
		}
	}
}

//����һ��qt������debug��ס��slot_fontSizeChange�������ᵼ�²۴���2�Σ�����ס����releaseģʽֱ���ܣ����ᴥ��2��
void QtLangSet::slot_fontSizeChange(int v)
{
	//��ʱ���÷��
	if (m_selectLexer != nullptr)
	{
		//ȫ���޸ģ����������Ե����з������
		//ȫ���޸ģ����������Ե����з������
		if (m_isGlobelItem)
		{
			if (m_curStyleData.font.pointSize() != v)
			{
				m_curStyleData.font.setPointSize(v);
				//qDebug() << m_curStyleData.font.family();
				m_isStyleChange = true;
				m_selectLexer->setFont(m_curStyleData.font, m_selectStyleId);
				if (ui.useGbFontSize->isChecked())
				{
					slot_useAlobalFontSize(true);
				}
			}
		}
		else if (!ui.modifyAllFont->isChecked())
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


void QtLangSet::getCurUseLexerTags(QVector<QString>& tag)
{
	CCNotePad* pMainNote = dynamic_cast<CCNotePad*>(parent());
	if (pMainNote != nullptr)
	{
		pMainNote->getCurUseLexerTags(tag);
	}
}

//ʹ��ȫ�������趨�������Է��
void QtLangSet::updateAllLangeStyleWithGlobal(GLOBAL_STYLE_SET flag)
{
	QFont oldfont;

	for (int index = 0; index <= L_TXT; ++index)
	{
		QsciLexer *pLexer = ScintillaEditView::createLexer(index);
		if (nullptr != pLexer)
		{

			switch (flag)
			{
			case GLOBAL_FONT:
			{
				for (int i = 0; i <= 255; ++i)
				{
					if (!pLexer->description(i).isEmpty())
					{
						oldfont = pLexer->font(i);
						oldfont.setFamily(m_curStyleData.font.family());
						pLexer->setFont(oldfont, i);
					}
				}
			}
			break;
			case GLOBAL_FONT_SIZE:
			{
				for (int i = 0; i <= 255; ++i)
				{
					if (!pLexer->description(i).isEmpty())
					{
						oldfont = pLexer->font(i);
						oldfont.setPointSize(m_curStyleData.font.pointSize());
						pLexer->setFont(oldfont, i);
					}
				}
			}
			break;
			case GLOBAL_FONT_BOLD:
			{
				for (int i = 0; i <= 255; ++i)
				{
					if (!pLexer->description(i).isEmpty())
					{
						oldfont = pLexer->font(i);
						oldfont.setBold(m_curStyleData.font.bold());
						pLexer->setFont(oldfont, i);
					}
				}
			}
			break;
			case GLOBAL_FONT_UNDERLINE:
			{
				for (int i = 0; i <= 255; ++i)
				{
					if (!pLexer->description(i).isEmpty())
					{
						oldfont = pLexer->font(i);
						oldfont.setUnderline(m_curStyleData.font.underline());
						pLexer->setFont(oldfont, i);
					}
				}
			}
			break;
			case GLOBAL_FONT_ITALIC:
			{
				for (int i = 0; i <= 255; ++i)
				{
					if (!pLexer->description(i).isEmpty())
					{
						oldfont = pLexer->font(i);
						oldfont.setItalic(m_curStyleData.font.italic());
						pLexer->setFont(oldfont, i);
					}
				}
			}
			break;
			case GLOBAL_FG_COLOR:
			{
				pLexer->setColor(m_curStyleData.color, -1);
			}
			break;
			case GLOBAL_BK_COLOR:
			{
				pLexer->setPaper(m_curStyleData.paper, -1);
				//Ĭ��ֽ����ɫ���QPalette����һ�¡�������Ҫ��������һ��
				pLexer->setDefaultPaper(m_curStyleData.paper);
			}
			break;
			default:
				break;
			}
			
			saveLangeSet(pLexer);
		}
		delete pLexer;
	}
}

//�ָ��������Եĳ�ʼ���á���restoreOriginLangOneStyle���ƣ��������ȸ���
void  QtLangSet::restoreOriginLangAllStyle()
{
	QsciLexer *pLexer = nullptr;

	//һ�����ã���ǰ�޸������������档������⵱ǰ�ĸո����ã��ֱ�����
	m_isStyleChange = false;

	for (int index = 0; index <= L_TXT; ++index)
	{

		pLexer = ScintillaEditView::createLexer(index);

		if (pLexer == nullptr)
		{
			continue;
		}

		//��������Զ�������ã�ֱ��ȫ��ɾ����
		QString cfgPath = QString("notepad/userstyle/%1/%2").arg(StyleSet::getCurrentStyle()).arg(pLexer->lexerTag());
		QSettings qs(QSettings::IniFormat, QSettings::UserScope, cfgPath);

		if (QFile::exists(qs.fileName()))
		{
			QFile::remove(qs.fileName());
		}

		delete pLexer;
		pLexer = nullptr;
	}
}

//ֻ��restoreOriginLangOneStyle�е���
static void restoreLangFontFgColorToDarkStyle(QsciLexer *pLexer, int i)
{
	if (QtLangSet::s_darkColorMap.contains(pLexer->color(i).name()))
	{
		pLexer->setColor(QtLangSet::s_darkColorMap.value(pLexer->color(i).name()), i);
	}
	}

//ֻ��restoreOriginLangOneStyle�е���
static void restoreLangPaperColorToDarkStyle(QsciLexer *pLexer, int i)
{
	//ע�����������ɫ��void StyleSet::setBlackStyle()�б���һ��
	//����Ҳ��ɰ���ɫ
	if (StyleSet::foldbgColor != pLexer->paper(i))
	{
		pLexer->setPaper(StyleSet::foldbgColor, i);
		pLexer->setDefaultPaper(StyleSet::foldbgColor);
	}
}

//�ָ��������Եĳ�ʼ����,ֻ�ָ�GLOBAL_STYLE_SETָ���ķ��
void QtLangSet::restoreOriginLangOneStyle(GLOBAL_STYLE_SET flag)
{

	QFont oldfont;
	QFont curfont;

	QColor oldClor;

	for (int index = 0; index <= L_TXT; ++index)
	{
		QsciLexer *pLexer = ScintillaEditView::createLexer(index);
		QsciLexer *pOriginLexer = ScintillaEditView::createLexer(index,"",true);
		if (nullptr != pLexer)
		{
			for (int i = 0; i <= 255; ++i)
			{
				if (!pLexer->description(i).isEmpty())
				{

					switch (flag)
					{
					case GLOBAL_FONT:
					{
						oldfont = pOriginLexer->font(i);
						pLexer->setFont(oldfont, i);
					}
					break;
					case GLOBAL_FONT_SIZE:
					{
						oldfont = pOriginLexer->font(i);
						curfont = pLexer->font(i);
						curfont.setPointSize(oldfont.pointSize());
						pLexer->setFont(curfont, i);
					}
					break;
					case GLOBAL_FONT_BOLD:
					{
						oldfont = pOriginLexer->font(i);
						curfont = pLexer->font(i);
						curfont.setBold(oldfont.bold());
						pLexer->setFont(curfont, i);
					}
					break;
					case GLOBAL_FONT_UNDERLINE:
					{
						oldfont = pOriginLexer->font(i);
						curfont = pLexer->font(i);
						curfont.setUnderline(oldfont.underline());
						pLexer->setFont(curfont, i);
					}
					break;
					case GLOBAL_FONT_ITALIC:
					{
						oldfont = pOriginLexer->font(i);
						curfont = pLexer->font(i);
						curfont.setItalic(oldfont.italic());
						pLexer->setFont(curfont, i);
					}
					break;
					case GLOBAL_FG_COLOR:
					{
						oldClor = pOriginLexer->color(i);
						pLexer->setColor(oldClor, i);

						if (BLACK_SE == StyleSet::getCurrentSytleId())
						{
							restoreLangFontFgColorToDarkStyle(pLexer, i);
					}

					}
					break;
					case GLOBAL_BK_COLOR:
					{
						oldClor = pOriginLexer->paper(i);
						pLexer->setPaper(oldClor, i);

						if (BLACK_SE == StyleSet::getCurrentSytleId())
						{
						restoreLangPaperColorToDarkStyle(pLexer, i);
					}
					}
					break;
					default:
						break;
					}
				}
			}
			saveLangeSet(pLexer);
		}
		delete pLexer;
		delete pOriginLexer;
	}
}
//Ԥ��ȫ���޸�����Ч�����ѵ�ǰ���е��﷨��������嶼�޸�һ��
void QtLangSet::previewAllGoblalChange()
{
	QVector<QString> tags;

	getCurUseLexerTags(tags);

	for (int i = 0, s = tags.size(); i < s; ++i)
	{
		emit viewLexerChange(tags.at(i));
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
				m_curStyleData.font.setFamily(font.family());
				m_selectLexer->setFont(m_curStyleData.font, m_selectStyleId);
				m_isStyleChange = true;

				if (m_isGlobelItem)
				{
					if (ui.useGbFont->isChecked())
					{
						slot_useAlobalFont(true);
					}
				}
				else
				{
					emit viewStyleChange(m_selectLexer->lexerTag(), m_selectStyleId, m_curStyleData.color, m_curStyleData.paper, m_curStyleData.font, true);
				}
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
			QListWidgetItem *item = new QListWidgetItem(langName);
			item->setData(Qt::UserRole,i);
			ui.langListWidget->addItem(item);
			delete pLexer;
		}
	}

	//��������Ƕ������ӵģ����ӳ�ִ�С������ǵ���ѡ���ѡ���У����ǵ�һ������asp��
	ui.langListWidget->sortItems(Qt::AscendingOrder);

	//�ֶ��޸ĵ�һ������Ϊ���ġ�
	if (ui.langListWidget->item(0)->text() == QString("AllGlobal"))
	{
		ui.langListWidget->item(0)->setText(tr("AllGlobal"));
}
}

//isLoadToUI�Ƿ������ʾ����ǰUI����
bool QtLangSet::readLangSetFile(QString langName,QString &keyword, QString &motherLang, QString & extTypes)
{
	QString userLangFile = QString("notepad/userlang/%1").arg(langName);//�Զ��������в�����.�ַ�����������д�����Ҫ���
	QSettings qs(QSettings::IniFormat, QSettings::UserScope, userLangFile);
	qs.setIniCodec("UTF-8");
	//qDebug() << qs.fileName();

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
		//ֻ�е�ǰ�Ƿ�ȫ�֣�����Ҫ��ʾ�û��Ƿ�Ҫ���֡�
		//ȫ�ֵĲ���Ҫ��ʾ����Ϊ�Ѿ�������ȫ��������ˡ�
		if (!m_isGlobelItem)
		{
			if (QMessageBox::Yes == QMessageBox::question(this, tr("Save Change"), tr("%1 style configuration has been modified. Do you want to save it?").arg(m_previousSysLangItem->text())))
			{
				saveCurLangSettings();
			}
		}
	}

	if (!item->isSelected())
	{
		item->setSelected(true);
	}

	ui.styleListWidget->clear();

	if (m_selectLexer != nullptr)
	{
		delete m_selectLexer;
		m_selectLexer = nullptr;
	}


	int lexId = item->data(Qt::UserRole).toInt();

	//ȫ�ֵ����Ҫ�����Դ�����ʱû���κη��
	if (lexId == L_GLOBAL)
	{
		m_isGlobelItem = true;
		ui.stackedWidget->setCurrentIndex(1);

		//ȫ���޸�ģʽ�£�Ŀǰ��ʱ�������޸����е�˵����Ŀǰ��ֻ��ȫ���޸�
		ui.modiryAllColor->setChecked(false);
		ui.modifyAllFont->setChecked(false);
		ui.modiryAllColor->setEnabled(false);
		ui.modifyAllFont->setEnabled(false);
	}
	else
	{
		m_isGlobelItem = false;
		ui.stackedWidget->setCurrentIndex(0);

		if (!ui.modiryAllColor->isEnabled())
		{
			ui.modiryAllColor->setEnabled(true);
		}
		if (!ui.modifyAllFont->isEnabled())
		{
			ui.modifyAllFont->setEnabled(true);
		}
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
		slot_styleItemSelect(itemtemp);
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
		slot_styleItemSelect(itemtemp);
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

	//�϶�������ȫ��������
	if(m_isGlobelItem || (ui.stackedWidget->currentIndex() == 1))
	{
		m_isGlobelItem = false;
		ui.stackedWidget->setCurrentIndex(0);

		if (!ui.modiryAllColor->isEnabled())
		{
			ui.modiryAllColor->setEnabled(true);
		}
		if (!ui.modifyAllFont->isEnabled())
		{
			ui.modifyAllFont->setEnabled(true);
		}
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
void QtLangSet::slot_styleItemSelect(QListWidgetItem *current)
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
	//�޸�����ᴥ���ۺ���ִ�У�����ȹر�һ���ź�
	enableFontChangeSensitive(false);

	ui.fontComboBox->setCurrentFont(font);
	ui.fontSpinBox->setValue(font.pointSize());

	ui.boldCheckBox->setChecked(font.bold());
	ui.italicCheckBox->setChecked(font.italic());
	ui.underlineCheckBox->setChecked(font.underline());

	fillForegroundColor(fcColor);
	fillBackgroundColor(bkColor);

	enableFontChangeSensitive(true);
	
}

//��ȡ�ض����Ե�����
bool QtLangSet::readLangSettings(QsciLexer *lexer, QString tag)
{
	QString cfgPath = QString("notepad/userstyle/%1/%2").arg(StyleSet::getCurrentStyle()).arg(tag);

	QSettings qs(QSettings::IniFormat, QSettings::UserScope, cfgPath);
	if (QFile::exists(qs.fileName()))
	{
		return lexer->readSettings(qs);
	}
	return false;
}


//����һ�����Ե����á�
void QtLangSet::saveLangeSet(QsciLexer *lexer)
{
	if (lexer != nullptr)
	{
		QString tag = lexer->lexerTag();

		QString cfgPath =  QString("notepad/userstyle/%1/%2").arg(StyleSet::getCurrentStyle()).arg(tag);

		QSettings qs(QSettings::IniFormat, QSettings::UserScope, cfgPath);
		lexer->writeSettings(qs);
		qs.sync();
	}
}

//���浱ǰѡ�����Ե�����
bool QtLangSet::saveCurLangSettings()
{
	if (m_isStyleChange && m_selectLexer != nullptr)
	{
		saveLangeSet(m_selectLexer);
		m_isStyleChange = false;
		return true;
	}
	return false;
}

//�޸�ǰ��ɫ
void QtLangSet::slot_changeFgColor()
{
	QColor color = QColorDialog::getColor(m_curStyleData.color, this, tr("Style Foreground Color"));
	if (color.isValid())
	{
		//ȫ���޸ģ����������Ե����з������
		if (m_isGlobelItem)
		{
			if (m_curStyleData.color != color)
			{
				m_curStyleData.color = color;
				fillForegroundColor(color);
				m_isStyleChange = true;

				//��ʱ���÷��
				if (m_selectLexer != nullptr)
				{
					m_selectLexer->setColor(color, m_selectStyleId);
				}

				if (ui.useGbFc->isChecked())
				{
					slot_useAlobalFgColor(true);
				}
			}
			
		}
		else if (!ui.modiryAllColor->isChecked())
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
				/*int row = ui.styleListWidget->count();

				QListWidgetItem* item = nullptr;

				for (int i = 0; i < row; ++i)
				{
					item = ui.styleListWidget->item(i);
					int styleId = item->data(Qt::UserRole).toInt();*/
					m_selectLexer->setColor(color, -1);
				/*}*/

				saveCurLangSettings();
				emit viewLexerChange(m_selectLexer->lexerTag());
			}
			
		}
	}
	
}

//�޸ı���ɫ
void QtLangSet::slot_changeBkColor()
{
	QColor color = QColorDialog::getColor(m_curStyleData.paper, this, tr("Style Background Color"));
	if (color.isValid())
	{
		//ȫ���޸ģ����������Ե����з������
		if (m_isGlobelItem)
		{
			if (m_curStyleData.paper != color)
			{
				m_curStyleData.paper = color;
				fillBackgroundColor(color);
				m_isStyleChange = true;

				//��ʱ���÷��
				if (m_selectLexer != nullptr)
				{
					m_selectLexer->setPaper(color, m_selectStyleId);
				}

				if (ui.useGbBc->isChecked())
				{
					slot_useAlobalBkColor(true);
				}
			}

		}
		else if (!ui.modiryAllColor->isChecked())
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
		else
		{
			//ȫ�������ɫ�޸�
			//��ǰǰ��ɫ�Ƿ�仯
			m_curStyleData.paper = color;
			fillBackgroundColor(color);
			m_isStyleChange = true;

			//��ʱ���÷��
			if (m_selectLexer != nullptr)
			{
				m_selectLexer->setPaper(color, -1);
				
				saveCurLangSettings();
				emit viewLexerChange(m_selectLexer->lexerTag());
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
		if (!m_isGlobelItem)
		{
			if (QMessageBox::Yes != QMessageBox::question(this, tr("Reset Style"), tr("Are you sure to reset language %1 sytle").arg(m_selectLexer->lexerTag())))
			{
				return;
			}

			//һ�����ã���ǰ�޸������������档������⵱ǰ�ĸո����ã��ֱ�����
			m_isStyleChange = false;

			//��������Զ�������ã�Ҳɾ����
			QString cfgPath = QString("notepad/userstyle/%1/%2").arg(StyleSet::getCurrentStyle()).arg(m_selectLexer->lexerTag());

			QSettings qs(QSettings::IniFormat, QSettings::UserScope, cfgPath);
			if (QFile::exists(qs.fileName()))
			{
				QFile::remove(qs.fileName());
			}

			//����ǰ�ɫ��񣬻���Ҫ����һ�����滻��ɫ�²����۵���ɫ
			if (BLACK_SE == StyleSet::getCurrentSytleId())
			{
				setLangFontFgColorToDarkStyle((LangType)m_selectLexer->lexerId(), m_selectLexer->lexerTag());
			}

			//һ��Ҫ�ȱ��棬��ΪselectInitLangTag����ܻ�仯

			int row = ui.styleListWidget->currentRow();

			selectInitLangTag(m_selectLexer->lexerTag());

			//�ֶ�ˢ��UI
			QListWidgetItem* styleItem = ui.styleListWidget->item(row);

			syncShowStyleItemToUI(styleItem);

			emit viewLexerChange(m_selectLexer->lexerTag());
		}
		else
		{
			//�����ȫ�����ã�����������Զ�����
			if (QMessageBox::Yes != QMessageBox::question(this, tr("Reset All Style"), tr("Are you sure to reset All language sytle")))
			{
				return;
			}
			restoreOriginLangAllStyle();

			//����ǰ�ɫ��񣬻���Ҫ����һ�����滻��ɫ�²����۵���ɫ
			if (BLACK_SE == StyleSet::getCurrentSytleId())
			{
				setAllLangFontFgColorToDarkStyle();
			}

			previewAllGoblalChange();

			//�ֶ�ˢ��UI��ȫ��Ŀǰֻ��һ��ȫ������

			if (m_selectLexer != nullptr)
			{
				delete m_selectLexer;
			}

			//����ǰ��ڷ�����ܶ�ȡԴ�ָ�;�ǰ������ȡԴ
			m_selectLexer  = ScintillaEditView::createLexer(L_GLOBAL,"",(BLACK_SE != StyleSet::getCurrentSytleId()));

			QListWidgetItem* styleItem = ui.styleListWidget->item(0);

			syncShowStyleItemToUI(styleItem);
		}
	}
}


void QtLangSet::slot_useAlobalFgColor(bool check)
{
	if (check)
	{
		updateAllLangeStyleWithGlobal(GLOBAL_FG_COLOR);
		previewAllGoblalChange();
	}
	else
	{
		restoreOriginLangOneStyle(GLOBAL_FG_COLOR);
		previewAllGoblalChange();
	}

}

void QtLangSet::slot_useAlobalBkColor(bool check)
{
	if (check)
	{
		updateAllLangeStyleWithGlobal(GLOBAL_BK_COLOR);
		previewAllGoblalChange();
	}
	else
	{
		restoreOriginLangOneStyle(GLOBAL_BK_COLOR);
		previewAllGoblalChange();
	}
}

void QtLangSet::slot_useAlobalFont(bool check)
{
	if (check)
	{
		updateAllLangeStyleWithGlobal(GLOBAL_FONT);
		previewAllGoblalChange();
	}
	else
	{
		restoreOriginLangOneStyle(GLOBAL_FONT);
		previewAllGoblalChange();
	}
}

void QtLangSet::slot_useAlobalFontSize(bool check)
{
	if (check)
	{
		updateAllLangeStyleWithGlobal(GLOBAL_FONT_SIZE);
		previewAllGoblalChange();
	}
	else
	{
		restoreOriginLangOneStyle(GLOBAL_FONT_SIZE);
		previewAllGoblalChange();
	}
}

void QtLangSet::slot_useAlobalFontBold(bool check)
{
	if (check)
	{
		updateAllLangeStyleWithGlobal(GLOBAL_FONT_BOLD);
		previewAllGoblalChange();
	}
	else
	{
		restoreOriginLangOneStyle(GLOBAL_FONT_BOLD);
		previewAllGoblalChange();
	}
}

void QtLangSet::slot_useAlobalFontUnderline(bool check)
{
	if (check)
	{
		updateAllLangeStyleWithGlobal(GLOBAL_FONT_UNDERLINE);
		previewAllGoblalChange();
	}
	else
	{
		restoreOriginLangOneStyle(GLOBAL_FONT_UNDERLINE);
		previewAllGoblalChange();
	}
}

void QtLangSet::slot_useAlobalFontItalic(bool check)
{
	if (check)
	{
		updateAllLangeStyleWithGlobal(GLOBAL_FONT_ITALIC);
		previewAllGoblalChange();
	}
	else
	{
		restoreOriginLangOneStyle(GLOBAL_FONT_ITALIC);
		previewAllGoblalChange();
	}
}



//��Ĭ�����Է��Ĳ�����ɫ���滻Ϊ��ɫ�����۵���ɫ
//��������ǿ��Դ����Զ������Եġ�
void QtLangSet::setLangFontFgColorToDarkStyle(LangType langId, QString tag)
{
	//QColor blackColor(Qt::black);
	//QColor lightColor(0xdedede);

	//QColor blackColor1(0x0000ff);
	//QColor lightColor1(0xffaa00);


	////C++ע�͵�Ĭ����ɫ��������
	//QColor blackColor2(0x007f00);
	//QColor lightColor2(0x009000);

	//QColor blackColor3(0x7f7f00);
	//QColor lightColor3(0xfca287);


	//QColor blackColor4(0x8000ff);
	//QColor lightColor4(0xffaa00);

	//ע�����������ɫ��void StyleSet::setBlackStyle()�б���һ��
	//QColor bkPaperColor(0, 0, 0);

	QsciLexer *pLexer = ScintillaEditView::createLexer(langId, tag);
		if (nullptr != pLexer)
		{
			for (int i = 0; i <= 255; ++i)
			{
				if (!pLexer->description(i).isEmpty())
				{
				/*if (blackColor == pLexer->color(i))
					{
					pLexer->setColor(lightColor, i);
					}
					else if (blackColor1 == pLexer->color(i))
					{
						pLexer->setColor(lightColor1, i);
					}
				else if (blackColor2 == pLexer->color(i))
				{
					pLexer->setColor(lightColor2, i);
				}
				else if (blackColor3 == pLexer->color(i))
				{
					pLexer->setColor(lightColor3, i);
				}
				else if (blackColor4 == pLexer->color(i))
				{
					pLexer->setColor(lightColor4, i);
				}*/

				if (QtLangSet::s_darkColorMap.contains(pLexer->color(i).name()))
				{
					pLexer->setColor(QtLangSet::s_darkColorMap.value(pLexer->color(i).name()), i);
				}

				//����Ҳ��ɰ���ɫ
				if (StyleSet::foldbgColor != pLexer->paper(i))
				{
					pLexer->setPaper(StyleSet::foldbgColor, i);
				}
			}
		}
		//Ĭ��ֽ����ɫ���QPalette����һ�¡�������Ϊ��ɫ��ɫ���䱳����QPalette��һ����������Ҫ��������һ��
		pLexer->setDefaultPaper(StyleSet::foldbgColor);
			saveLangeSet(pLexer);
			delete pLexer;
		}
	}

//�Ѱ�ɫϵ���治���׿������ɫ���滻һ�¡�Ĭ�����з��ģ�����Ĭ�Ϸ������ɫϵ�ġ�
//�ѷ���0x000000�����壬���0xdedede
//����0x0000ff����ɫ�����0xffff00
//�������û�д����Զ������ԡ�
void QtLangSet::setAllLangFontFgColorToDarkStyle()
{
	initDarkColorMap();

	for (int index = 0; index <= L_TXT; ++index)
	{
		setLangFontFgColorToDarkStyle((LangType)index);
					}
					}

//��鵱ǰ�Ƿ��Ѿ����ڰ���ɫ��������
bool QtLangSet::isExistDarkLangSetings()
{
	QString cfgPath = QString("notepad/userstyle/black/AllGlobal");
	QSettings qs(QSettings::IniFormat, QSettings::UserScope, cfgPath);
	return QFile::exists(qs.fileName());
				}

////�ѷ���0xdedede�����壬���0x000000
//void QtLangSet::setAllLangFontFgColorToLightStyle()
//{
//	QColor blackColor(Qt::black);
//	QColor lightColor(0xdedede);
//
//	QColor blackColor1(0xffaa00);
//	QColor lightColor1(0x0000ff);
//
//	for (int index = 0; index <= L_TXT; ++index)
//	{
//		QsciLexer *pLexer = ScintillaEditView::createLexer(index);
//		if (nullptr != pLexer)
//		{
//			for (int i = 0; i <= 255; ++i)
//			{
//				if (!pLexer->description(i).isEmpty())
//				{
//					if (lightColor == pLexer->color(i))
//					{
//						pLexer->setColor(blackColor, i);
//					}
//					else if (blackColor1 == pLexer->color(i))
//					{
//						pLexer->setColor(lightColor1, i);
//					}
//				}
//			}
//			saveLangeSet(pLexer);
//			delete pLexer;
//		}
//	}
//}
