#include "std.zh"

ffc script TriggerOnCursor
{
    void run(int combo, int screen_index)
    {
		Trace(Region->Width);
		Trace(Region->Height);
		Trace(Region->ScreenWidth);
		Trace(Region->ScreenHeight);
		Trace(Region->NumCombos);

        while (true)
        {
			int x = Input->Mouse[MOUSE_X];
			int y = Input->Mouse[MOUSE_Y];
			Screen->DrawCombo(3, x, y,
				combo,
				1,
				1,
				3,
				-1, -1, 0, 0, 0, 0, 0, true, OP_OPAQUE);

			// TODO
			// x += Viewport->X;
			// y += Viewport->Y;
			if (this->X < x && this->Y < y && this->X + 32 >= x && this->Y + 32 >= y)
			{
				Screen->TriggerSecrets(screen_index);
			}

            Waitframe();
        }
    }
}
