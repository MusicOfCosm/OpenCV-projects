// Astuces pour bien démarrer : 
//   1. Utilisez la fenêtre Explorateur de solutions pour ajouter des fichiers et les gérer.
//   2. Utilisez la fenêtre Team Explorer pour vous connecter au contrôle de code source.
//   3. Utilisez la fenêtre Sortie pour voir la sortie de la génération et d'autres messages.
//   4. Utilisez la fenêtre Liste d'erreurs pour voir les erreurs.
//   5. Accédez à Projet > Ajouter un nouvel élément pour créer des fichiers de code, ou à Projet > Ajouter un élément existant pour ajouter des fichiers de code existants au projet.
//   6. Pour rouvrir ce projet plus tard, accédez à Fichier > Ouvrir > Projet et sélectionnez le fichier .sln.

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <cmath>
#include <ctime>
#include <thread>
#include <random>
#include <vector>

unsigned int seed = (unsigned int)time(0);
std::mt19937 gen(seed);

const float pi = 3.1415927f;

//float randomize() { //Between 0 and 1
//    //return double(rand()) / RAND_MAX;
//}

int randint(int min, int max) {
    //double rtrn_val = (max - min) * randomize() + min; //Mine
    //double rtrn_val = min + (rand() % static_cast<int>(max - min + 1)); //Internet
    //return round(rtrn_val);
    std::uniform_int_distribution<int> rdint(min, max);
    return rdint(gen);
}

float uniform(float min, float max) {
    //double rtrn_val = (max - min) * randomize() + min; //Mine
    //float rtrn_val = min + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(max-min))); //Internet
    //return rtrn_val;
    std::uniform_real_distribution<float> rdfloat(min, max);
    return rdfloat(gen);
}

//Variables
static const int HEIGHT = 1080; //720HD
static const int WIDTH  = 1920; //1280HD
static int NUMBER = 100000; //randint(((HEIGHT* WIDTH) / 100) * 5, ((HEIGHT* WIDTH) / 100) * 20); //5 percent of pixels
static float SPEED = uniform(1.f, 2.5f);
static int SENSOR_RANGE = randint(5, 30); // Sensor Offset
static int SENSOR_SIZE = randint(0, 2);
static float SA = uniform(30.0f, 60.0f) * pi / 180.f; //uniform(pi / 3.5f, pi / 2.0f); //Sensor Angle
static float RA = uniform(5.0f, 60.0f) * pi / 180.f; //uniform(0, pi / 4.5f); //Rotation Angle
static bool BOUNDARY = randint(0, 1); //0 and 1 are false and true
static int START = randint(0, 3); //0: random, 1: centre, 2: circle, 3: corner
static bool BLUR = randint(0, 1);
static int SPECIES = randint(1, 3);
static bool IGNORE = randint(0, 1);


cv::Mat Canvas = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3); //The canvas showing the colored output, init with three color channels (BGR) set to zero (black)
cv::Mat Gray[3] = { //All three species have their own grayscale trail that they follow
    cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1),
    cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1),
    cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1)
};
cv::Mat Gray_ref[3] = { //Where the species will place their pixels (other than the Canvas), it will be used to make the Grays they follow
    cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1),
    cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1),
    cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1)
};
cv::Mat trail_color_values[3] = /*cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3);*/ {
    cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3),
    cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3),
    cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3)
};
cv::Mat Grey; //Grayscaled Canvas, allows the three Grays to communicate with their gray_val

uint8_t gray_val[3], color[3][3];

float corner_X;
float corner_Y;

class Agent {
public:
    float x, y, orientation;
    
    //Constructor (basically __init__)
    Agent() {
        if (START == 0) { //random, the matrix goes from 0 to the designated values-1
            x = uniform(0, float(WIDTH-1));
            y = uniform(0, float(HEIGHT-1));
            orientation = uniform(-pi, pi);
        }
        else if (START == 1){ //centre
            x = WIDTH / 2.0f;
            y = HEIGHT / 2.0f;
            orientation = uniform(-pi, pi);
        }
        else if (START == 2) { //circle
            float tmp_pos = uniform(-pi, pi);
            x = WIDTH  / 2.0f + cos(tmp_pos) * uniform(0, float(HEIGHT) / 2.f);
            y = HEIGHT / 2.0f + sin(tmp_pos) * uniform(0, float(HEIGHT-1) / 2.f);
            orientation = (float)acos(  (float(WIDTH)/2.f - x) / sqrt( pow((float(WIDTH) / 2.f - x), 2) + pow((float(HEIGHT) / 2.f - y), 2))  );
            if (y > float(HEIGHT) / 2.f) { orientation = -orientation; }
        }
        else { //corner
            x = (BOUNDARY == true) ? float(randint(0, 1) * (WIDTH - 1)) : 0.F;
            y = (BOUNDARY == true) ? float(randint(0, 1) * (HEIGHT - 1)) : 0.F;
            orientation = uniform(-pi, pi);
        }
    }

private:
    template<typename coord>
    void Boundary(coord& x, coord& y, int Width, int Height) {
        if (BOUNDARY == true) {
            if (x > Width) {
                x = coord(Width);
            }
            else if (x < 0) {
                x = 0;
            }

            if (y > Height) {
                y = coord(Height);
            }
            else if (y < 0) {
                y = 0;
            }
        }

        else {
            if (x > Width) {
                x -= Width;
            }
            else if (x < 0) {
                x += Width;
            }

            if (y > Height) {
                y -= Height;
            }
            else if (y < 0) {
                y += Height;
            }
        }
    }

    int sense(Agent& self, float sensorAngleOffset, int index) {
        float sensorAngle = self.orientation + sensorAngleOffset;
        float sensorDirX = cos(sensorAngle);
        float sensorDirY = sin(sensorAngle);
        int sensorCentreX = int(self.x + sensorDirX * SENSOR_RANGE);
        int sensorCentreY = int(self.y + sensorDirY * SENSOR_RANGE);
        int sum = 0;

        for (int offsetX = -SENSOR_SIZE; offsetX <= SENSOR_SIZE; offsetX++) { //Goes from -SENSOR_SIZE to SENSOR_SIZE
            for (int offsetY = -SENSOR_SIZE; offsetY <= SENSOR_SIZE; offsetY++) {
                int posX = sensorCentreX + offsetX;
                int posY = sensorCentreY + offsetY;
                Boundary(posX, posY, WIDTH, HEIGHT); //resetting posX and posY to be inside the bounds

                for (int i = 0; i < SPECIES; i++)
                {
                    if (Gray_ref[i].at<uint8_t>(posY, posX) == Gray_ref[index].at<uint8_t>(posY, posX)) {
                        sum += Gray_ref[i].at<uint8_t>(posY, posX) * (255 / gray_val[index]); //Order is important!!
                    }
                    else if (IGNORE == false) { sum -= Gray_ref[i].at<uint8_t>(posY, posX) * (255 / gray_val[index]); }
                }
            }
        }

        return sum;
    }

public:
    void movement(Agent& self, int index, int number) {
        
        if (
            Grey.at<uint8_t>(int(round(self.y)), int(round(self.x))) == gray_val[0] ||
            Grey.at<uint8_t>(int(round(self.y)), int(round(self.x))) == gray_val[1] ||
            Grey.at<uint8_t>(int(round(self.y)), int(round(self.x))) == gray_val[2]
            )
            { self.orientation = uniform(-pi, pi); }
        
        //Placing pixel on canvas
        else if (SPECIES && number <= NUMBER && number != 0)
        {
            Canvas.at<cv::Vec3b>(int(round(self.y)), int(round(self.x))) = { color[index][0], color[index][1], color[index][2] };
            Gray_ref[index].at<uint8_t>(int(round(self.y)), int(round(self.x))) = gray_val[index]; //Went from Gray to Gray_ref

            //Sensing other pixels
            int forward = sense(self, 0.0f, index);
            int left = sense(self, SA, index);
            int right = sense(self, -SA, index);

            if (forward >= left && forward >= right) {
                ;
            }

            else if (forward < left && forward < right) {
                self.orientation += uniform(-RA, RA);
            }

            else if (left > right) {
                self.orientation += RA;
            }

            else if (left < right) {
                self.orientation += -RA;
            }
        }
        
        //moving
        self.x += SPEED * cos(self.orientation);
        self.y += SPEED * sin(self.orientation);
        if (self.x < 0 || self.x > WIDTH - 1 || self.y < 0 || self.y > HEIGHT - 1) {
            if (BOUNDARY == true) {
                self.orientation = uniform(-pi, pi);
            }
            Boundary(self.x, self.y, WIDTH - 1, HEIGHT-1);
        }
    }
};

//int value = Gray[i].at<uint8_t>(posY, posX);
//
//if (Gray[i].at<uint8_t>(posY, posX) == Gray[index].at<uint8_t>(posY, posX)) {
//    sum += value * 250 / gray_val[index]; //Order is important!!
//}
//else { sum -= value * 250 / gray_val[index]; }

int main(int argc, char** argv) 
{
    for (int i = 0; i < 3; i++) {
        while (color[i][0] + color[i][1] + color[i][2] < 200) {
            color[i][0] = randint(0, 255);
            color[i][1] = randint(0, 255);
            color[i][2] = randint(0, 255);
        }
    }

    if (BOUNDARY == true && START == 3) {
        corner_X = float(randint(0, 1) * (WIDTH - 1));
        corner_Y = float(randint(0, 1) * (HEIGHT - 1));
    }

    gray_val[0] = (color[0][0] + color[0][1] + color[0][2]) / 3;
    gray_val[1] = (color[0][0] + color[0][1] + color[0][2]) / 3;
    gray_val[2] = (color[0][0] + color[0][1] + color[0][2]) / 3;

    std::cout << "\tINITIAL VALUES:\n" <<
        "\nNumber: " << NUMBER <<
        "\nSpeed: " << SPEED <<
        "\nSensor range: " << SENSOR_RANGE <<
        "\nSensor size: " << SENSOR_SIZE <<
        "\nBoundaries (n: 0, y: 1): " << BOUNDARY <<
        "\nSensor Angle: " << SA * 180 / pi << '\370'/* \370 is ° */ <<
        "\nRotation Angle: " << RA * 180 / pi << '\370' <<
        "\nStart (0: random, 1: centre, 2: circle, 3: corner): " << START <<
        "\nBlur (n: 0, y: 1): " << BLUR << 
        "\nSpecies ignore each other (n: 0, y: 1): " << IGNORE << '\n';
        for (int i = 0; i < 3; i++) {
            std::cout << "Color of species " << i + 1 << " - B:" << (int)color[i][0] << ", G: " << (int)color[i][1] << ", R: " << (int)color[i][2] << "\n";
        }
        std::cout << "\n\n";


    //Making a window for the canvas
    cv::namedWindow("Canvas", cv::WINDOW_NORMAL);
    cv::setWindowProperty("Canvas", cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);

    // Showing the image
    cv::imshow("Canvas", Canvas);
    cv::cvtColor(Canvas, Grey, cv::COLOR_BGR2GRAY);


    //Initiating agents
    //std::vector<Agent> agents;
    Agent* agents = new Agent[1000000];
    for (int i = 0; i < 1000000; i++) {
        agents[i] = Agent();
    }
    //Agent agents[SIZE];
    //for (int i = 0; i < SIZE; i++) {
    //    agents[i] = Agent();
    //}

    //initiating variables that measure performance and set fps cap
    clock_t start, finish;
    float iter_time;
    uint iter = 1;
    float total = 0.0f, total_average = 0.0f, average = 0.0f;
    int limit = 30;
    float cap = 1.f/limit;
    float diff;

    int color_index = 0;
    bool pause = false;
    bool winVars = false;
    bool loop = true;
    //Simulation loop
    while (loop){
        //(cv::pollKey() & 0xFF) == 27
        do
        {
            int key = cv::pollKey();
            switch (key)
            {
                case 27: //Esc
                case 69: //E
                case 101: //e
                case 81: //Q
                case 113: //q
                    loop = false;
                    break;

                case 102: //f or F
                case 70:  //same as f || F but switch cases are a bit special
                    if (cv::getWindowProperty("Canvas", (uint8_t)cv::WND_PROP_FULLSCREEN) == (uint8_t)cv::WINDOW_NORMAL)
                    {
                        cv::setWindowProperty("Canvas", (uint8_t)cv::WND_PROP_FULLSCREEN, (uint8_t)cv::WINDOW_FULLSCREEN);
                    } //using uint8_t so that it takes less space (I bet it changes nothing)
                    else if (cv::getWindowProperty("Canvas", (uint8_t)cv::WND_PROP_FULLSCREEN) == (uint8_t)cv::WINDOW_FULLSCREEN)
                    {
                        cv::setWindowProperty("Canvas", (uint8_t)cv::WND_PROP_FULLSCREEN, (uint8_t)cv::WINDOW_NORMAL);
                        cv::resizeWindow("Canvas", 640, 360);
                    }
                    break;

                case 32: //spacebar
                    if (pause == true) { pause = false; }
                    else if (pause == false) { pause = true; }
                    break;

                case 118: //v
                case 86:  //V
                    if (winVars == false)
                    {
                        cv::namedWindow("Variables", cv::WINDOW_NORMAL);
                        cv::resizeWindow("Variables", 720, 720);
                        cv::moveWindow("Variables", 0, 0);

                        // NUMBER
                        int NumberVal = (int)round(NUMBER / 1000);
                        cv::createTrackbar("Number*1k", "Variables", NULL, 1000);
                        cv::setTrackbarPos("Number*1k", "Variables", NumberVal);

                        // SPEED
                        int SpeedVal = (int)round(SPEED * 100);
                        cv::createTrackbar("Speed/100", "Variables", NULL, 500);
                        cv::setTrackbarPos("Speed/100", "Variables", SpeedVal);

                        // SENSOR_RANGE
                        cv::createTrackbar("Sense Dist", "Variables", NULL, 50);
                        cv::setTrackbarPos("Sense Dist", "Variables", SENSOR_RANGE);

                        // SENSOR_SIZE
                        cv::createTrackbar("Sense Size", "Variables", NULL, 5);
                        cv::setTrackbarPos("Sense Size", "Variables", SENSOR_SIZE);

                        // SA or Sensor Angle
                        cv::createTrackbar("Sense Ang", "Variables", NULL, 180);
                        cv::setTrackbarPos("Sense Ang", "Variables", (int)round(SA * 180 / pi));

                        // RA or Rotation Angle
                        cv::createTrackbar("Turn Angle", "Variables", NULL, 180);
                        cv::setTrackbarPos("Turn Angle", "Variables", (int)round(RA * 180 / pi));

                        //SPECIES
                        cv::createTrackbar("Species", "Variables", NULL, 3);
                        cv::setTrackbarPos("Species", "Variables", SPECIES);

                        // color[3][3]
                        cv::createTrackbar("Species n\370", "Variables", NULL, 2);
                        cv::setTrackbarPos("Species n\370", "Variables", color_index);
                        cv::createTrackbar("Red", "Variables", NULL, 255);
                        cv::setTrackbarPos("Red", "Variables", color[color_index][2]);
                        cv::createTrackbar("Green", "Variables", NULL, 255);
                        cv::setTrackbarPos("Green", "Variables", color[color_index][1]);
                        cv::createTrackbar("Blue", "Variables", NULL, 255);
                        cv::setTrackbarPos("Blue", "Variables", color[color_index][0]);

                        //int limit = 60;
                        //float cap = float(1 / limit);
                        cv::createTrackbar("fps cap", "Variables", NULL, 120);
                        cv::setTrackbarPos("fps cap", "Variables", limit);

                        // BOUNDARY: 0 and 1 are false and true
                        cv::createTrackbar("Boundaries", "Variables", NULL, 1);
                        cv::setTrackbarPos("Boundaries", "Variables", BOUNDARY);

                        // BLUR
                        cv::createTrackbar("Blur", "Variables", NULL, 1);
                        cv::setTrackbarPos("Blur", "Variables", BLUR);

                        //IGNORE
                        cv::createTrackbar("Ignore", "Variables", NULL, 1);
                        cv::setTrackbarPos("Ignore", "Variables", IGNORE);

                        //pause
                        cv::createTrackbar("Pause", "Variables", NULL, 1);
                        cv::setTrackbarPos("Pause", "Variables", pause);

                        //cv::createTrackbar("Apply", "Variables", NULL, 1);
                        winVars = true;
                    }
                    else if (winVars == true) {
                        cv::destroyWindow("Variables");
                        winVars = false;
                    }
                    break;

                //default:
                //    if (key != -1) { std::cout << key << '\n'; }
            }

            if (winVars == true) {
                //if (cv::getTrackbarPos("Apply", "Variables") == 1 || key == 13 ) { //13 is Enter (carriage return actually)
                NUMBER = cv::getTrackbarPos("Number*1k", "Variables") * 1000;

                SPEED = cv::getTrackbarPos("Speed/100", "Variables") / 100.f;

                SENSOR_RANGE = cv::getTrackbarPos("Sense Dist", "Variables");

                SENSOR_SIZE = cv::getTrackbarPos("Sense Size", "Variables");

                SA = cv::getTrackbarPos("Sense Ang", "Variables") * pi / 180.f; //Sensor Angle

                RA = cv::getTrackbarPos("Turn Angle", "Variables") * pi / 180.f; //Rotation Angle

                BOUNDARY = cv::getTrackbarPos("Boundaries", "Variables"); //0 and 1 are false and true

                BLUR = cv::getTrackbarPos("Blur", "Variables");

                IGNORE = cv::getTrackbarPos("Ignore", "Variables");

                SPECIES = cv::getTrackbarPos("Species", "Variables");

                color[color_index][2] = cv::getTrackbarPos("Red", "Variables");
                color[color_index][1] = cv::getTrackbarPos("Green", "Variables");
                color[color_index][0] = cv::getTrackbarPos("Blue", "Variables");
                color_index = cv::getTrackbarPos("Species n\370", "Variables");
                cv::setTrackbarPos("Red", "Variables", color[color_index][2]);
                cv::setTrackbarPos("Green", "Variables", color[color_index][1]);
                cv::setTrackbarPos("Blue", "Variables", color[color_index][0]);

                //Got to update limit, otherwise, remaking the var window will put cap at the initial value
                limit = cv::getTrackbarPos("fps cap", "Variables");
                cap = 1.f / limit;

                cv::setTrackbarPos("Pause", "Variables", pause); //Need this when I press the button while the var window is up
                pause = cv::getTrackbarPos("Pause", "Variables");
                //cv::setTrackbarPos("Apply", "Variables", 0);
            }

            //cv::Mat Gray[3] = {
            //    cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1), //All three species have their on grayscale trail that they follow
            //    cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1),
            //    cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1)
            //};
        } while (pause == true);

        start = clock();

        cv::cvtColor(Canvas, Grey, cv::COLOR_BGR2GRAY);

        //Canvas.setTo(cv::Scalar(0, 0, 0));

        //Decreases brightness, giving a fading effect
        Canvas.convertTo(Canvas, -1, 1, -1);
        for (int i = 0; i < 3; i++){ 
            Gray_ref[i].convertTo(Gray_ref[i], -1, 1, -1);
            //Gray[i] = Gray_ref[i];
        }

        if (BLUR == true) {
            cv::GaussianBlur(Canvas, Canvas, cv::Size(3, 3), 0);
            for (int i = 0; i < SPECIES; i++)
                { cv::GaussianBlur(Gray_ref[i], Gray_ref[i], cv::Size(3, 3), 0); }
            //cv::GaussianBlur(trail_color_values[0], trail_color_values[0], cv::Size(3, 3), 0);
            //cv::GaussianBlur(trail_color_values[1], trail_color_values[1], cv::Size(3, 3), 0);
            //cv::GaussianBlur(trail_color_values[2], trail_color_values[2], cv::Size(3, 3), 0);
        }

        //if (SPECIES) {
        int n = 0;
        for (n; n < SPECIES; n++) {
            for (int i = n * (NUMBER / SPECIES); i < (n + 1) * (NUMBER / SPECIES); i++) {
                agents[i].movement(agents[i], n, i);
            }
        }
        for (int i = (SPECIES == 0) ? 0 : n * (NUMBER / SPECIES); i < 1000000; i++) {
            agents[i].movement(agents[i], NULL, i);
        };
        //}
        //else {
        //    for (int i = 0; i < 1000000; i++) {
        //        agents[i].movement(agents[i], NULL, NULL);
        //    }
        //}

        //for (uint8_t iter = 0; iter < 3; iter++) {
        //    for (int i = 0; i < NUMBER; i++) {
        //        agents[i].movement(agents[i]);
        //    }
        //    if (blur == true) {
        //        cv::GaussianBlur(Canvas, Canvas, cv::Size(3, 3), 0);
        //    }
        //}

        cv::imshow("Canvas", Canvas);

        finish = clock();
        iter_time = float(finish - start) / CLOCKS_PER_SEC;
        //std::cout << limit << '\n' << cap << '\n' << iter_time << "\n\n";
        if (iter_time < cap) {
            diff = cap - iter_time;
            //the *1000 converts seconds to milliseconds, I learned way too late that the nano version doesn't work on Windows
            std::this_thread::sleep_for(std::chrono::milliseconds( int(diff * 1000) ));
        }

        finish = clock();
        iter_time = float(finish - start) / CLOCKS_PER_SEC;

        total += iter_time;
        total_average = total/iter;
        
        //std::cout << "\rAverage: " << 1 / average << " fps" << " or " << average * 1000 << " ms per frame     ";
        printf("\rAverage: %f fps or %f ms per frame      ", 1 / iter_time, iter_time * 1000); // \r\033[1A
        //printf("\nTotal average: %f fps or %f ms per frame      ", 1 / total_average, total_average * 1000); //%f for float, %d is int
        iter++;
    }

    //std::cout << "\nAverage: " << 1 / mean << " fps" << " or " << mean << " seconds per frame" << '\n';
    cv::destroyAllWindows();
    delete[] agents;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}
