#include "initServer.h"

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x - 1;
    coord.Y = y - 1;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void decorate()
{
    int x = 10, y = 2;
    gotoxy(x, y++);
    cout << "::::::::::: ::::::::::     :::     ::::    ::::    ::::::::::: :::    ::: ::::    ::::  :::::::::  ";
    gotoxy(x, y++);
    cout << "    :+:     :+:          :+: :+:   +:+:+: :+:+:+       :+:     :+:   :+:  +:+:+: :+:+:+ :+:    :+: ";
    gotoxy(x, y++);
    cout << "    +:+     +:+         +:+   +:+  +:+ +:+:+ +:+       +:+     +:+  +:+   +:+ +:+:+ +:+ +:+    +:+ ";
    gotoxy(x, y++);
    cout << "    +#+     +#++:++#   +#++:++#++: +#+  +:+  +#+       +#+     +#++:++    +#+  +:+  +#+ +#++:++#:  ";
    gotoxy(x, y++);
    cout << "    +#+     +#+        +#+     +#+ +#+       +#+       +#+     +#+  +#+   +#+       +#+ +#+    +#+ ";
    gotoxy(x, y++);
    cout << "    #+#     #+#        #+#     #+# #+#       #+#       #+#     #+#   #+#  #+#       #+# #+#    #+# ";
    gotoxy(x, y++);
    cout << "    ###     ########## ###     ### ###       ###       ###     ###    ### ###       ### ###    ### ";
    gotoxy(1, y + 2);
}