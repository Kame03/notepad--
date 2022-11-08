#include "extlexermanager.h"

//ר�����������û��Զ����Ext �� �ʷ�Lexer�������ࡣ
//����һ���ļ���ext��׺�����ٸ�֪��ʹ��ʲôlexer�����﷨����

ExtLexerManager* ExtLexerManager::s_instance = nullptr;

ExtLexerManager::ExtLexerManager()
{
}

ExtLexerManager * ExtLexerManager::getInstance()
{
	if (s_instance == nullptr)
	{
		s_instance = new ExtLexerManager();
	}
	return s_instance;
}

ExtLexerManager::~ExtLexerManager()
{
	m_extToLexerIdMap.clear();
}

int ExtLexerManager::size()
{
	return m_extToLexerIdMap.size();
}

bool ExtLexerManager::contains(QString ext)
{
	return m_extToLexerIdMap.contains(ext);
}

void ExtLexerManager::remove(QString ext)
{
	if (m_extToLexerIdMap.contains(ext))
	{
		m_extToLexerIdMap.remove(ext);
	}
}


//ext:�ļ��ĺ�׺�� langTagName:���Ե�tag���� 
//lexerId �﷨��id��������û��Զ��壬���Ȼ��L_USER_TXT,L_USER_CPP,L_USER_HTML,L_USER_JS,L_USER_XML, L_USER_INI �е�һ����
//langTagName:ֻ�����û����������£�����ҪtagName����Ϊ���û������lexer����tagname�ǹ̶��ġ�
void ExtLexerManager::addNewExtType(QString ext, LangType lexerId, QString langTagName)
{
	LexerInfo value(lexerId, langTagName);
	m_extToLexerIdMap.insert(ext,value);
}

bool ExtLexerManager::getLexerTypeByExt(QString ext, LexerInfo& lexer)
{
	if (m_extToLexerIdMap.contains(ext))
	{
		lexer = m_extToLexerIdMap[ext];
		return true;
	}
	return false;
}
