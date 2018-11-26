﻿#include "pch.h"
#include <iostream>

using namespace std;

int main()
{
	db::btree* tr = new db::btree();
	int a[500] = { 321, 226, 50, 347, 339, 109, 167, 33, 311, 45, 292, 354, 179, 225, 361,
		213, 397, 264, 280, 53, 139, 407, 152, 147, 358, 176, 295, 39, 12, 101, 270, 237,
		77, 353, 430, 72, 106, 93, 439, 368, 471, 236, 25, 427, 96, 298, 483, 154, 46, 60,
		105, 178, 300, 265, 301, 41, 386, 444, 465, 91, 64, 1, 406, 104, 32, 424, 377, 141,
		359, 464, 485, 261, 498, 205, 382, 190, 185, 148, 2, 19, 242, 192, 23, 374, 445, 455,
		306, 393, 335, 385, 387, 416, 492, 170, 207, 357, 463, 487, 142, 164, 180, 459, 222,
		448, 229, 87, 30, 474, 24, 488, 80, 256, 210, 310, 440, 489, 132, 419, 428, 396, 327,
		248, 95, 478, 490, 496, 215, 330, 189, 244, 29, 274, 367, 246, 491, 249, 195, 162,
		499, 272, 322, 486, 314, 398, 184, 43, 289, 263, 449, 408, 328, 308, 315, 433, 16, 90,
		307, 159, 140, 204, 348, 395, 426, 69, 118, 331, 457, 467, 497, 296, 42, 342, 352,
		121, 399, 281, 411, 254, 145, 128, 71, 188, 38, 351, 451, 304, 338, 421, 435, 70,
		243, 493, 299, 135, 11, 40, 18, 400, 82, 341, 287, 127, 94, 238, 92, 48, 174, 450,
		163, 108, 275, 434, 452, 211, 412, 363, 131, 216, 126, 312, 7, 389, 422, 268, 113, 65,
		323, 201, 362, 392, 34, 75, 329, 282, 47, 232, 375, 73, 379, 425, 313, 209, 155,
		153, 183, 107, 326, 495, 115, 62, 191, 220, 381, 172, 250, 316, 252, 31, 333, 320,
		144, 247, 22, 99, 202, 84, 343, 214, 157, 288, 181, 258, 165, 97, 51, 122, 394, 199,
		376, 418, 208, 317, 44, 390, 21, 112, 240, 472, 102, 441, 37, 143, 372, 469, 350, 431,
		14, 297, 259, 151, 285, 171, 423, 74, 124, 9, 417, 136, 461, 470, 149, 13, 277, 35,
		364, 203, 409, 217, 66, 27, 10, 253, 401, 4, 278, 480, 158, 58, 378, 286, 59, 318,
		134, 78, 432, 402, 227, 345, 119, 355, 114, 384, 231, 283, 116, 447, 373, 233, 55,
		370, 150, 443, 125, 324, 482, 284, 466, 369, 269, 219, 494, 98, 442, 200, 239, 245,
		319, 28, 156, 360, 291, 337, 293, 198, 26, 133, 89, 303, 120, 279, 477, 218, 404, 453,
		8, 86, 391, 346, 415, 267, 76, 177, 403, 52, 349, 473, 290, 446, 206, 437, 20, 332,
		309, 420, 500, 365, 388, 83, 271, 111, 186, 241, 336, 460, 161, 475, 255, 251, 234,
		130, 173, 230, 266, 340, 17, 194, 79, 260, 414, 123, 235, 257, 276, 117, 56, 262, 193,
		305, 458, 88, 85, 175, 413, 138, 212, 15, 3, 468, 484, 5, 160, 366, 454, 129, 228,
		479, 36, 67, 476, 166, 103, 405, 187, 462, 302, 68, 61, 429, 481, 168, 221, 410,
		344, 436, 438, 223, 137, 356, 383, 273, 110, 81, 6, 182, 63, 325, 456, 294, 371,
		146, 57, 380, 196, 224, 100, 54, 169, 49, 197, 334 };
	tr->create(a, 500);
	tr->print_tree();
	for (int i = 0;i < 500;i++) {
		if ((i + 1) % 100 == 0) 
			tr->print_tree(); //99 199 .. 499
		cout << a[i] << endl;

		//		if (a[i] == 109) {
		//			cout << endl;
		//		}
		tr->delkey(a[i]);
		//if (i > 180) tr->print_tree();
	}
	tr->insert(10,1111);
	//	tr->delkey(42);
	//	tr->delkey(43);
	//	tr->print_tree();
		//tr->print_leaf();
		//cout << tr->search(89) << "," << tr->search(21) << "," << tr->search(24) << endl;
}