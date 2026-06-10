#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS 20
#define COLS 50
#define MAX_SHAPES 100

// Shape types
typedef enum {
    SHAPE_LINE,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

// Shape data structures
typedef struct {
    int x1, y1;
    int x2, y2;
} Line;

typedef struct {
    int x, y; // Top-left corner
    int width, height;
} Rectangle;

typedef struct {
    int cx, cy; // Center
    int radius;
} Circle;

typedef struct {
    int x1, y1;
    int x2, y2;
    int x3, y3;
} Triangle;

// General shape structure
typedef struct {
    int id;
    ShapeType type;
    union {
        Line line;
        Rectangle rect;
        Circle circle;
        Triangle triangle;
    } data;
} Shape;

// Global state
Shape shapes[MAX_SHAPES];
int num_shapes = 0;
int next_id = 1;
char canvas[ROWS][COLS];

// Helper: robust input reader for integers with boundary validation
int get_int_input(const char *prompt, int min_val, int max_val) {
    char buffer[100];
    int value;
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Error reading input. Please try again.\n");
            continue;
        }
        // Remove trailing newline
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (sscanf(buffer, "%d", &value) != 1) {
            printf("Invalid input. Please enter an integer.\n");
            continue;
        }
        if (value < min_val || value > max_val) {
            printf("Out of bounds. Please enter a value between %d and %d.\n", min_val, max_val);
            continue;
        }
        return value;
    }
}

// Helper: robust input reader for integers with default fallback
int get_int_input_default(const char *prompt, int default_val, int min_val, int max_val) {
    char buffer[100];
    int value;
    while (1) {
        printf("%s [%d]: ", prompt, default_val);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return default_val;
        }
        // Remove trailing newline
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (strlen(buffer) == 0) {
            return default_val;
        }
        
        if (sscanf(buffer, "%d", &value) != 1) {
            printf("Invalid input. Please enter an integer.\n");
            continue;
        }
        if (value < min_val || value > max_val) {
            printf("Out of bounds. Please enter a value between %d and %d.\n", min_val, max_val);
            continue;
        }
        return value;
    }
}

// Initialize canvas with underscores
void clear_canvas() {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            canvas[r][c] = '_';
        }
    }
}

// Plot a single point with boundary protection
void draw_point(int x, int y) {
    if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {
        canvas[y][x] = '*';
    }
}

// Bresenham's line drawing algorithm
void draw_line(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = -abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (1) {
        draw_point(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Draw rectangle using line segments or direct plotting
void draw_rectangle(int x, int y, int w, int h) {
    // Top & Bottom edges
    for (int i = 0; i < w; i++) {
        int px = x + i;
        if (px >= 0 && px < COLS) {
            if (y >= 0 && y < ROWS) canvas[y][px] = '*';
            if (y + h - 1 >= 0 && y + h - 1 < ROWS) canvas[y + h - 1][px] = '*';
        }
    }
    // Left & Right edges
    for (int i = 0; i < h; i++) {
        int py = y + i;
        if (py >= 0 && py < ROWS) {
            if (x >= 0 && x < COLS) canvas[py][x] = '*';
            if (x + w - 1 >= 0 && x + w - 1 < COLS) canvas[py][x + w - 1] = '*';
        }
    }
}

// Midpoint circle algorithm
void draw_circle(int cx, int cy, int r) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    // Plot symmetric points helper
    #define PLOT8(cx, cy, x, y) \
        do { \
            draw_point(cx + x, cy + y); \
            draw_point(cx - x, cy + y); \
            draw_point(cx + x, cy - y); \
            draw_point(cx - x, cy - y); \
            draw_point(cx + y, cy + x); \
            draw_point(cx - y, cy + x); \
            draw_point(cx + y, cy - x); \
            draw_point(cx - y, cy - x); \
        } while (0)

    PLOT8(cx, cy, x, y);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        PLOT8(cx, cy, x, y);
    }
    #undef PLOT8
}

// Draw triangle using 3 lines
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line(x1, y1, x2, y2);
    draw_line(x2, y2, x3, y3);
    draw_line(x3, y3, x1, y1);
}

// Redraw canvas with all active shapes
void render_canvas() {
    clear_canvas();
    for (int i = 0; i < num_shapes; i++) {
        Shape s = shapes[i];
        switch (s.type) {
            case SHAPE_LINE:
                draw_line(s.data.line.x1, s.data.line.y1, s.data.line.x2, s.data.line.y2);
                break;
            case SHAPE_RECTANGLE:
                draw_rectangle(s.data.rect.x, s.data.rect.y, s.data.rect.width, s.data.rect.height);
                break;
            case SHAPE_CIRCLE:
                draw_circle(s.data.circle.cx, s.data.circle.cy, s.data.circle.radius);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle(s.data.triangle.x1, s.data.triangle.y1,
                              s.data.triangle.x2, s.data.triangle.y2,
                              s.data.triangle.x3, s.data.triangle.y3);
                break;
        }
    }
}

// Display canvas to console with coordinate indices
void display_canvas() {
    printf("\n");
    // Print column headers (tens line)
    printf("   ");
    for (int x = 0; x < COLS; x++) {
        printf("%d", x / 10);
    }
    printf("\n");

    // Print column headers (ones line)
    printf("   ");
    for (int x = 0; x < COLS; x++) {
        printf("%d", x % 10);
    }
    printf("\n");

    // Print rows with row index prefix
    for (int y = 0; y < ROWS; y++) {
        printf("%02d ", y);
        for (int x = 0; x < COLS; x++) {
            printf("%c", canvas[y][x]);
        }
        printf("\n");
    }
    printf("\n");
}

// Print details of a single shape
void print_shape_details(Shape s) {
    switch (s.type) {
        case SHAPE_LINE:
            printf("ID %d: Line from (%d, %d) to (%d, %d)\n", s.id, s.data.line.x1, s.data.line.y1, s.data.line.x2, s.data.line.y2);
            break;
        case SHAPE_RECTANGLE:
            printf("ID %d: Rectangle at top-left (%d, %d), width %d, height %d\n", s.id, s.data.rect.x, s.data.rect.y, s.data.rect.width, s.data.rect.height);
            break;
        case SHAPE_CIRCLE:
            printf("ID %d: Circle center (%d, %d), radius %d\n", s.id, s.data.circle.cx, s.data.circle.cy, s.data.circle.radius);
            break;
        case SHAPE_TRIANGLE:
            printf("ID %d: Triangle vertices (%d, %d), (%d, %d), (%d, %d)\n", s.id, s.data.triangle.x1, s.data.triangle.y1, s.data.triangle.x2, s.data.triangle.y2, s.data.triangle.x3, s.data.triangle.y3);
            break;
    }
}

// List all active shapes
void list_shapes() {
    if (num_shapes == 0) {
        printf("No shapes currently in the picture.\n");
        return;
    }
    printf("\n=== Active Shapes ===\n");
    for (int i = 0; i < num_shapes; i++) {
        print_shape_details(shapes[i]);
    }
    printf("=====================\n\n");
}

// Add shape flow
void add_shape_flow() {
    if (num_shapes >= MAX_SHAPES) {
        printf("Error: Canvas is full. Delete some shapes first.\n");
        return;
    }

    printf("\n--- Add a Shape ---\n");
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Circle\n");
    printf("4. Triangle\n");
    int type_choice = get_int_input("Enter shape type (1-4): ", 1, 4);

    Shape new_shape;
    new_shape.id = next_id++;

    switch (type_choice) {
        case 1:
            new_shape.type = SHAPE_LINE;
            printf("Enter coordinates for the start and end of the line:\n");
            new_shape.data.line.x1 = get_int_input("x1 (column, 0-49): ", 0, COLS - 1);
            new_shape.data.line.y1 = get_int_input("y1 (row, 0-19): ", 0, ROWS - 1);
            new_shape.data.line.x2 = get_int_input("x2 (column, 0-49): ", 0, COLS - 1);
            new_shape.data.line.y2 = get_int_input("y2 (row, 0-19): ", 0, ROWS - 1);
            break;
        case 2:
            new_shape.type = SHAPE_RECTANGLE;
            printf("Enter parameters for the rectangle:\n");
            new_shape.data.rect.x = get_int_input("Top-left x (column, 0-49): ", 0, COLS - 1);
            new_shape.data.rect.y = get_int_input("Top-left y (row, 0-19): ", 0, ROWS - 1);
            new_shape.data.rect.width = get_int_input("Width (1-50): ", 1, COLS);
            new_shape.data.rect.height = get_int_input("Height (1-20): ", 1, ROWS);
            break;
        case 3:
            new_shape.type = SHAPE_CIRCLE;
            printf("Enter parameters for the circle:\n");
            new_shape.data.circle.cx = get_int_input("Center x (column, 0-49): ", 0, COLS - 1);
            new_shape.data.circle.cy = get_int_input("Center y (row, 0-19): ", 0, ROWS - 1);
            new_shape.data.circle.radius = get_int_input("Radius (0-50): ", 0, COLS);
            break;
        case 4:
            new_shape.type = SHAPE_TRIANGLE;
            printf("Enter coordinates for the three vertices of the triangle:\n");
            new_shape.data.triangle.x1 = get_int_input("x1 (column, 0-49): ", 0, COLS - 1);
            new_shape.data.triangle.y1 = get_int_input("y1 (row, 0-19): ", 0, ROWS - 1);
            new_shape.data.triangle.x2 = get_int_input("x2 (column, 0-49): ", 0, COLS - 1);
            new_shape.data.triangle.y2 = get_int_input("y2 (row, 0-19): ", 0, ROWS - 1);
            new_shape.data.triangle.x3 = get_int_input("x3 (column, 0-49): ", 0, COLS - 1);
            new_shape.data.triangle.y3 = get_int_input("y3 (row, 0-19): ", 0, ROWS - 1);
            break;
    }

    shapes[num_shapes++] = new_shape;
    printf("Shape added successfully with ID %d!\n", new_shape.id);
    
    // Automatically render and show canvas
    render_canvas();
    display_canvas();
}

// Delete shape flow
void delete_shape_flow() {
    if (num_shapes == 0) {
        printf("No shapes to delete.\n");
        return;
    }

    list_shapes();
    int id_to_delete = get_int_input("Enter the ID of the shape to delete: ", 1, next_id - 1);

    int found_idx = -1;
    for (int i = 0; i < num_shapes; i++) {
        if (shapes[i].id == id_to_delete) {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1) {
        printf("Error: Shape with ID %d not found.\n", id_to_delete);
        return;
    }

    // Shift remaining shapes
    for (int i = found_idx; i < num_shapes - 1; i++) {
        shapes[i] = shapes[i + 1];
    }
    num_shapes--;

    printf("Shape with ID %d deleted successfully!\n", id_to_delete);
    
    // Automatically render and show canvas
    render_canvas();
    display_canvas();
}

// Modify shape flow
void modify_shape_flow() {
    if (num_shapes == 0) {
        printf("No shapes to modify.\n");
        return;
    }

    list_shapes();
    int id_to_modify = get_int_input("Enter the ID of the shape to modify: ", 1, next_id - 1);

    int found_idx = -1;
    for (int i = 0; i < num_shapes; i++) {
        if (shapes[i].id == id_to_modify) {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1) {
        printf("Error: Shape with ID %d not found.\n", id_to_modify);
        return;
    }

    Shape *s = &shapes[found_idx];
    printf("\nModifying shape details:\n");
    print_shape_details(*s);
    printf("Enter new values (press Enter to keep current):\n");

    switch (s->type) {
        case SHAPE_LINE:
            s->data.line.x1 = get_int_input_default("x1", s->data.line.x1, 0, COLS - 1);
            s->data.line.y1 = get_int_input_default("y1", s->data.line.y1, 0, ROWS - 1);
            s->data.line.x2 = get_int_input_default("x2", s->data.line.x2, 0, COLS - 1);
            s->data.line.y2 = get_int_input_default("y2", s->data.line.y2, 0, ROWS - 1);
            break;
        case SHAPE_RECTANGLE:
            s->data.rect.x = get_int_input_default("Top-left x", s->data.rect.x, 0, COLS - 1);
            s->data.rect.y = get_int_input_default("Top-left y", s->data.rect.y, 0, ROWS - 1);
            s->data.rect.width = get_int_input_default("Width", s->data.rect.width, 1, COLS);
            s->data.rect.height = get_int_input_default("Height", s->data.rect.height, 1, ROWS);
            break;
        case SHAPE_CIRCLE:
            s->data.circle.cx = get_int_input_default("Center x", s->data.circle.cx, 0, COLS - 1);
            s->data.circle.cy = get_int_input_default("Center y", s->data.circle.cy, 0, ROWS - 1);
            s->data.circle.radius = get_int_input_default("Radius", s->data.circle.radius, 0, COLS);
            break;
        case SHAPE_TRIANGLE:
            s->data.triangle.x1 = get_int_input_default("x1", s->data.triangle.x1, 0, COLS - 1);
            s->data.triangle.y1 = get_int_input_default("y1", s->data.triangle.y1, 0, ROWS - 1);
            s->data.triangle.x2 = get_int_input_default("x2", s->data.triangle.x2, 0, COLS - 1);
            s->data.triangle.y2 = get_int_input_default("y2", s->data.triangle.y2, 0, ROWS - 1);
            s->data.triangle.x3 = get_int_input_default("x3", s->data.triangle.x3, 0, COLS - 1);
            s->data.triangle.y3 = get_int_input_default("y3", s->data.triangle.y3, 0, ROWS - 1);
            break;
    }

    printf("Shape with ID %d modified successfully!\n", id_to_modify);
    
    // Automatically render and show canvas
    render_canvas();
    display_canvas();
}

int main() {
    clear_canvas();
    printf("=========================================\n");
    printf("   Welcome to the 2D Graphics Editor!    \n");
    printf("=========================================\n");
    
    while (1) {
        printf("\n=========================================\n");
        printf("               MAIN MENU                 \n");
        printf("=========================================\n");
        printf("1. Add a shape\n");
        printf("2. Delete a shape\n");
        printf("3. Modify a shape\n");
        printf("4. Display canvas\n");
        printf("5. List active shapes\n");
        printf("6. Exit\n");
        printf("=========================================\n");
        
        int choice = get_int_input("Choose an option (1-6): ", 1, 6);
        
        switch (choice) {
            case 1:
                add_shape_flow();
                break;
            case 2:
                delete_shape_flow();
                break;
            case 3:
                modify_shape_flow();
                break;
            case 4:
                render_canvas();
                display_canvas();
                break;
            case 5:
                list_shapes();
                break;
            case 6:
                printf("\nExiting. Thank you for using the Graphics Editor!\n");
                return 0;
        }
    }
}
