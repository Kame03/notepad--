#pragma once
#include <QSettings>
#include <QVariant>

static QString SKIN_KEY = "skinid";
static QString LANGS_KEY = "langs";
static QString AUTOWARP_KEY = "warp";
static QString INDENT_KEY = "indent";
static QString MAX_BIG_TEXT = "maxtsize";
static QString SOFT_STATUS = "rstatus";
static QString SOFT_KEY = "rkey";
static QString RESTORE_CLOSE_FILE = "restore"; //�ָ��ر�ʱ�򿪵��ļ�
static QString PRO_DIR = "prodir";//���������ļ���·��
//static QString RESTORE_SIZE = "rsize";//����ر�ʱ�Ĵ�С
static QString WIN_POS = "pos";//����ر��ǵĴ�С
static QString ICON_SIZE = "iconsize";//ͼ���С
static QString ZOOMVALUE = "zoom"; //�Ŵ���
static QString VERSION = "version";//��ǰ�汾
static QString FINDRESULTPOS = "findpos";//���Ҵ���������λ��

class NddSetting
{
public:
	static bool checkNoExistAdd(QString key, QVariant & value);

	static void init();

	static QString getKeyValueFromSets(QString key);

	static bool updataKeyValueFromSets(QString key, QString  value);

	static QByteArray getKeyByteArrayValue(QString key);

	static void updataKeyByteArrayValue(QString key, QByteArray & value);

	static void addKeyValueToSets(QString  key, QString  value);

	static int getKeyValueFromNumSets(const QString key);

	static bool updataKeyValueFromNumSets(const QString key, int value);

	static void addKeyValueToNumSets(QString key, int value);

	static void close();

	static bool isDbExist()
	{
		return s_isExistDb;
	}

private:

	static bool s_isExistDb;
	static bool s_isContentChanged;
	static int s_reference;

	static QSettings* s_nddSet;
};