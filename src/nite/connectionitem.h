#pragma once

#include <QtGui>
#include <QtXml>

class InputItem;
class OutputItem;
class ProcessorItem;

class ConnectionItem: public QGraphicsPathItem
{
public:
	ConnectionItem(InputItem* _to, OutputItem* _from);

	enum { Type = UserType + 4 };
	virtual int type() const { return Type; }

	OutputItem* from() const { return m_from; }

	void setValid(bool _v) { m_isValid = _v; }

	QPointF wouldAdjust() const;

	ProcessorItem* toProcessor() const;
	ProcessorItem* fromProcessor() const;

	static void fromDom(QDomElement& _element, QGraphicsScene* _scene);
	void saveYourself(QDomElement& _root, QDomDocument& _doc) const;

	void rejigEndPoints();

private:
	virtual void focusInEvent(QFocusEvent* _e);
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);


	bool m_isValid;
	OutputItem* m_from;
};

class MultipleConnectionItem: public QGraphicsPathItem
{
public:
	MultipleConnectionItem(ProcessorItem* _to, ProcessorItem* _from);

	enum { Type = UserType + 9 };
	virtual int type() const { return Type; }

	ProcessorItem* from() const { return m_from; }

	void setValid(bool _v) { m_isValid = _v; }

	ProcessorItem* toProcessor() const;
	ProcessorItem* fromProcessor() const;

	static void fromDom(QDomElement& _element, QGraphicsScene* _scene);
	void saveYourself(QDomElement& _root, QDomDocument& _doc) const;

	void rejigEndPoints();

private:
	virtual void focusInEvent(QFocusEvent* _e);
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);

	bool m_isValid;
	ProcessorItem* m_from;
};
