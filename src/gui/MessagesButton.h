#ifndef MESSAGESBUTTON_H
#define MESSAGESBUTTON_H

#include <QPushButton>

class QStyleOptionButton;

class MessagesButton : public QPushButton
{
public:
    MessagesButton( QWidget* parent = 0 );
	~MessagesButton();

	quint64 unreadCount = 0;

protected:
    void paintEvent(QPaintEvent *);

};

#endif // MESSAGESBUTTON_H