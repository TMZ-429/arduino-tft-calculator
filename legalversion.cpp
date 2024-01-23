# include <MCUFRIEND_kbv.h>
# include <TouchScreen.h>
# include <CLExpr.h>
# include <string.h>

# define BUFFER_SIZE 32

# define TOUCH_ORIENTATION LANDSCAPE
# define YP A2
# define XM A3
# define YM 8
# define XP 9

# define MAGENTA 0xF81F
# define YELLOW  0xFFE0
# define GREEN   0x07E0
# define BLACK   0x0000
# define WHITE   0xFFFF
# define CYAN    0x07FF
# define GREY    0x8410
# define BLUE    0x001F
# define RED     0xF800

# define TEXT_COLOUR WHITE

MCUFRIEND_kbv tft;
TouchScreen touchScreen = TouchScreen(XP, YP, XM, YM, 200);
TSPoint touchPoint;

int textSize = 2, coolDown = 0, graphingMode = 0, colourSwitch = 0;
char charset[65] = "0123456789\n+-*/^().\nqwertyuiopasdfghjklzxcvbnm<=  \u0018  [CLEAR]", buffer[BUFFER_SIZE], buffer0[BUFFER_SIZE],
* ioStrings[][2] = {
    /*Simple syntax for making a simple input/output command: the 1st line is the commmand input, the second is the output
    Feel free to add your own commands!*/
    {
        "help",
        "\n\n\n\n\n\n\
This calculator contains customizeable commands\n\
At the moment, the only command is the one that toggles graphing mode\n\
It can be enabled with 'grm'\n\
In graphing mode, you type out an equation like '12*x-86'\n\
The equation will be interpreted as 'y=12*x-86'\n\
"
    },
    {
        "example",
        "\n\n\n\n\n\
You can also make custom input/output commands if you ever\n\
Need to remember a formula or something.\n\
The syntax is simple, just add a comma at the end of these\n\
curly brackets, then add two things in quotation marks.\n\n\
The first phrase in quotations is the command you type to call\n\
the command\n\
The second is what the command outputs!"
    }
};

using namespace nCL;

/*
characters that can fit across a screen = (80 / textSize)
characters that can fit down a screen = (40 / textSize)
40x20
170x900 is the lower-left corner
buttons are ~30-50 pxls away from each other horizontally
buttons are ~30-50 pxls away from each other vertically but are shorter than they are wide (Due to space between newlines)
*/

void check_ESCMD() {
    //If C++ were more like NodeJS I would've made this system something more akin to the Cryptchat servers' chatting commmands
    if (!graphingMode) {
        //We don't use the standard input/output commands in graphing mode because there's a grid in the way
        for (char ** cmp : ioStrings) {
            if (!strcmp(buffer, cmp[0])) {
                tft.setTextSize(1);
                tft.println(cmp[1]);
                tft.setTextSize(2);
                return;
            }
        }
    }
    if (!strcmp(buffer, "grm")) {
        strcpy(buffer, "");
        /*
         The line in loop() right after calling this function returns if there is no buffer, this effectively stops the calculator
         from printing a random straight line or 0.00000000000000000000 after changing modes
        */
        virtualKeyboard();
        graphingMode = !graphingMode;
    } else if (!strcmp(buffer, "bmr")) {
        bomber();
    }
}

void bomber() {
   //Practical joke (Also my first Arduino project)
   tft.setCursor(0, 0);
   tft.fillScreen(GREEN);
   tft.setTextColor(BLACK);
   tft.setTextSize(2);
    char bombText[] = "THIS BOMB IS ABOUT TO DETONATE IN ";
    for (char letter : bombText) {
        tft.print(letter);
        delay(100);
    }
    for (int i = 5; i > -1; i--) {
        tft.fillScreen(GREEN);
        tft.setCursor(0, 0);
        tft.print(bombText);
        tft.print(i);
        delay(200);
    }
    tft.setTextColor(TEXT_COLOUR);
    virtualKeyboard();
}

void virtualKeyboard() {
   tft.fillScreen(BLACK);
   tft.setCursor(0, 240);
    for (char character : charset) {
        //I really wish there was a tft.printf()
        tft.print(" ");
        tft.write(character); //tft.write is for some reason the only way to print unicode characters
    }
    tft.setCursor(0, 0);
}

void drawGrid() {
    //virtualKeyboard(); //<-Description for why this is commmented is in the last comment in this function
    //I want a distinct & bright colour for printing the numbers on the grid, but also one that isn't being used already
    tft.setTextColor(TEXT_COLOUR != MAGENTA ? MAGENTA : CYAN);
    tft.setTextSize(1); //Text size needs to be as small as possible to fit numbers in the grid
    for (int x = 0, y = 230; x < 480; x += 20, y -= 20) {
        //We increment by 20 because incrementing by 10 takes up too much space when printing numbers on the X & Y axis
        tft.drawLine(x, 230, x, 0, GREY);
        tft.drawLine(0, y, 640, y, GREY);
        /*We draw a line straight upwards & straight forwards from our x & y positions respectively -- standard stuff.
        They're grey because that's distinct enough to be seen but not bright, because we want the red & blue pixels to be visible
        through the grey.*/
        tft.setCursor(x, 230);
        tft.print(x - 100);
        tft.setCursor(0, y);
        tft.print(x - 100);
    }
    tft.setTextColor(TEXT_COLOUR);
    tft.setTextSize(2);
    //Rewrite the buffer over the grid & numbers
    /*
    I was going to have virtualKeyboard() as the first line of this function so the equation you input would be written over the
    grid, but, I wanted to be able to draw multiple lines so you could view how they intersect or something if you wanted to;
    Rewriting the entire grid & deleting the previous graph while also removing traces of whatever was last input would sort of
    be counter-productive for viewing how multiple functions/whatever interact so I have it here commented
    Of course, doing this instead means that the grid writes slightly over the previous inputs but what can a man do?
    At least the grid's in grey and the text is white (unless you change the text colour) so they're distinct enough (probably)
    
    tft.setCursor(0, 0);
    tft.print(buffer);
    */
}

void graph() {
    drawGrid();
    /*I wanted to use malloc here to get a specific size
    but the minimum size of substitution is sizeof(buffer) + 3(instances of x)
    and getting the instances of x alone isn't worth whatever you could "save" by allocating the specific size with it */
    char substitution[64];
    for (int x = -100; x < 430; x++) {
        strcpy(substitution, "");
        for (char character : buffer) {
            if (character == 'x') {
                char sub[3];
                sprintf(sub, "%d", x);
                strcat(substitution, sub);
            } else {
                strncat(substitution, &character, 1);
            }
        }
        int y = EvalExpressionDouble(substitution, 0);
        colourSwitch = !colourSwitch;
        tft.drawPixel(x + 100, 130 - y, (colourSwitch ? RED : BLUE));
        /*X 100 is for some reason the left side, so we increase every X value by 100 so the first value is accurate to the grid
        Y - 130 because the Y axis on this screen rotation actually goes downwards when increased (Maybe setting screen rotation-
        -to 3 would set this back to normal? I'm fine with this workaround though -- no functional difference).*/
    }
    /*Pretty standard -- we start from -100, go 430 beyond that & substitute every value along the way in for x,
    use that as the x coordinate and the full equation is the y coordinate.
    Then we draw a pixel at (x, y)
    Also the pixels are drawn red if they're even & blue if they're odd
    */
}

void setup() {
    Serial.begin(9600);
    if (!Serial) { delay(5000); }
    uint16_t ID = tft.readID();
    tft.begin(ID);
    tft.setRotation(1);
    tft.setTextSize(textSize);
    tft.setTextColor(TEXT_COLOUR);
    virtualKeyboard();
}

void loop() {
    delay(1);
    if (coolDown > 0) {
        //Cooldown so that we don't input 77777 when we clicked 7 once and the pen gets dragged a micrometer
        coolDown--;
        return; 
    }
    touchPoint = touchScreen.getPoint();
    pinMode(YP, OUTPUT);
    pinMode(XM, OUTPUT);
    /*The Z axis is effectively pressure -- for some reason the touch screen detects pressures like -64 (How is that possible?)
    We want a decent amount of pressure before going forward just to make sure that for instance debris didn't brush up on the screen
    or air isn't blown on it and it triggers an input for whatever reason.
    */
    if (touchPoint.z > 100) {
        /*
        What we're doing here is taking the point we touched the screen & trying to match it up to an X & Y axis of the keyboard
        When we find a range of 50 pxls from the point we pressed, we divide our point by the amount of times we increment our value
        (we also subtract the offset) to get the position of the cursor in the charset array.
        */
        for (int i = 170; i < 940; i += 40) {
            int l = (i - 170) / 40, k = 0;
            if (touchPoint.x < i + 30 && touchPoint.x > i - 20) {
                for (int i = 900; i > 750; i -= 30) {
                    if (touchPoint.y > i - 20 && touchPoint.y < i + 30) {
                        k = ((i - 900) / 30) * -1;
                        /*
                        We return here because this would be trying to tap right next to 9 or next to .
                         (There's nothing there but due to the way it's programmed,
                         it'll keep going on the array)
                        */
                        if ((l > 9 && k == 4) || (l > 7 && k == 3)) { return; }
                        //This basically shifts our character to the next row & same column if we've tapped lower than another row
                        l += (k == 4 ? 0 : k == 3 ? 11 : k == 2 ? 20 : 40);
                        //This is for special characters such as backspace & enter
                        if (l > 45) {
                            if (l == 46) { //BACKSPACE
                                int bufferLength = strlen(buffer);
                                if (bufferLength < 2) {
                                    strcpy(buffer, "");
                                } else {
                                    memmove(&buffer[bufferLength - 1], &buffer[bufferLength], bufferLength - 1);
                                }
                                virtualKeyboard();
                                tft.print(buffer);
                            } else if (l == 47) { //ENTER
                                check_ESCMD(); //This function checks for special escape commands
                                if (!strcmp(buffer, "")) { return; } //Return if there is no input
                                if (!graphingMode) {
                                    double result_D = EvalExpressionDouble(buffer, 0);
                                    tft.print("\n> ");
                                    tft.println(result_D, 20);
                                } else {
                                    graph();
                                    tft.print("\n");
                                }
                                strcpy(buffer0, buffer);
                                strcpy(buffer, "");
                                coolDown = 100;
                            } else if (l == 50) { //UP_ARROW
                                strcpy(buffer, buffer0); //Switches the equation we're writing in with the last one we entered
                                virtualKeyboard();
                                tft.print(buffer);
                            } else if (l >= 53) { //CLEAR_SCREEN
                                strcpy(buffer, "");
                                strcpy(buffer0, "");
                                virtualKeyboard();
                            }
                            return;
                        }
                        strncat(buffer, &charset[l], 1);
                        tft.print(charset[l]);
                        coolDown = 125;
                        return;
                    }
                }
            }
        }
    }
}