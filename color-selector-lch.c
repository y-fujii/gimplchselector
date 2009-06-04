/*
	L*C*h* color selector for GIMP
	in the public domain by y.fujii <y-fujii@mimosa-pudica.net>
*/

#include <math.h>
#include <gtk/gtk.h>
#include <libgimpcolor/gimpcolor.h>
#include <libgimpmodule/gimpmodule.h>
#include <libgimpwidgets/gimpwidgets.h>


/* L*C*h* color space */

typedef struct ColorLch_ {
	double l, c, h;
} ColorLch;

void srgb_to_lch( ColorLch* lch, const GimpRGB* srgb ) {
	double sr = fmin( fmax( srgb->r, 0.0 ), 1.0 );
	double sg = fmin( fmax( srgb->g, 0.0 ), 1.0 );
	double sb = fmin( fmax( srgb->b, 0.0 ), 1.0 );
	double lr = pow( sr, 2.2 );
	double lg = pow( sg, 2.2 );
	double lb = pow( sb, 2.2 );
	double y = 0.2126 * lr + 0.7152 * lg + 0.0722 * lb;
	double x = (0.4124 / 0.9505) * lr + (0.3576 / 0.9505) * lg + (0.1805 / 0.9505) * lb;
	double z = (0.0193 / 1.0890) * lr + (0.1192 / 1.0890) * lg + (0.9505 / 1.0890) * lb;
	double fy = pow( y, 1.0 / 3.0 );
	double fx = pow( x, 1.0 / 3.0 );
	double fz = pow( z, 1.0 / 3.0 );
	double a = 5.0 * (fx - fy);
	double b = 2.0 * (fy - fz);
	lch->l = fy;
	lch->c = sqrt( a * a + b * b );
	lch->h = atan2( a, b );
}

void lch_to_srgb( GimpRGB* srgb, const ColorLch* lch ) {
	double a = lch->c * sin( lch->h );
	double b = lch->c * cos( lch->h );
	double fy = lch->l;
	double fx = fy + a / 5.0;
	double fz = fy - b / 2.0;
	double y = fy * fy * fy;
	double x = fx * fx * fx;
	double z = fz * fz * fz;
	double lr = ( 3.2410 * 0.9505) * x + (-1.5374) * y + (-0.4986 * 1.0890) * z;
	double lg = (-0.9692 * 0.9505) * x + ( 1.8760) * y + ( 0.0416 * 1.0890) * z;
	double lb = ( 0.0556 * 0.9505) * x + (-0.2040) * y + ( 1.0570 * 1.0890) * z;
	lr = fmin( fmax( lr, 0.0 ), 1.0 );
	lg = fmin( fmax( lg, 0.0 ), 1.0 );
	lb = fmin( fmax( lb, 0.0 ), 1.0 );
	printf( "%f, %f, %f\n", lr, lg, lb );
	srgb->r = pow( lr, 1.0 / 2.2 );
	srgb->g = pow( lg, 1.0 / 2.2 );
	srgb->b = pow( lb, 1.0 / 2.2 );
}


/* LchSelector class */

typedef struct LchSelector_ {
	GimpColorSelector super;
	GtkAdjustment* scale_l;
	GtkAdjustment* scale_c;
	GtkAdjustment* scale_h;
} LchSelector;

typedef struct LchSelectorClass_ {
	GimpColorSelectorClass super;
} LchSelectorClass;

G_DEFINE_DYNAMIC_TYPE( LchSelector, lch_selector, GIMP_TYPE_COLOR_SELECTOR );

static void lch_selector_set_color(
	GimpColorSelector* self,
	const GimpRGB* rgb,
	const GimpHSV* hsv
) {
	ColorLch lch;
	srgb_to_lch( &lch, rgb );

	gtk_adjustment_set_value( ((LchSelector*)self)->scale_l, lch.l * 100.0 );
	gtk_adjustment_set_value( ((LchSelector*)self)->scale_c, lch.c * 100.0 );
	gtk_adjustment_set_value( ((LchSelector*)self)->scale_h, lch.h * (180.0 / M_PI) );
}

static void lch_selector_on_value_changed( GtkAdjustment* adj, LchSelector* self ) {
	ColorLch lch;
	lch.l = self->scale_l->value / 100.0;
	lch.c = self->scale_c->value / 100.0;
	lch.h = self->scale_h->value * (M_PI / 180.0);
	lch_to_srgb( &GIMP_COLOR_SELECTOR( self )->rgb, &lch );
	gimp_rgb_to_hsv( &GIMP_COLOR_SELECTOR( self )->rgb, &GIMP_COLOR_SELECTOR( self )->hsv );

	gimp_color_selector_color_changed( GIMP_COLOR_SELECTOR( self ) );
}

static void lch_selector_class_init( LchSelectorClass* klass ) {
	GimpColorSelectorClass* super = GIMP_COLOR_SELECTOR_CLASS( klass );
	super->name      = "Lch";
	super->help_id   = "gimp-colorselector-lch";
	super->stock_id  = GTK_STOCK_COLOR_PICKER;
	super->set_color = lch_selector_set_color;
}

static void lch_selector_class_finalize( LchSelectorClass* klass ) {
}

static void lch_selector_init( LchSelector* self ) {
	GtkWidget* table = gtk_table_new( 3, 3, FALSE );
	gtk_box_pack_start( GTK_BOX( self ), table, TRUE, TRUE, 0 );

	self->scale_l = GTK_ADJUSTMENT( gimp_scale_entry_new(
		GTK_TABLE( table ), 0, 0, "L*", -1, -1,
		0.0, 0.0, 100.0, 1.0, 10.0,
		0, TRUE, 0.0, 0.0, "L*", NULL
	) );
	g_signal_connect(
		self->scale_l, "value-changed",
		G_CALLBACK( lch_selector_on_value_changed ), self
	);
	self->scale_c = GTK_ADJUSTMENT( gimp_scale_entry_new(
		GTK_TABLE( table ), 0, 1, "C*", -1, -1,
		0.0, 0.0, 100.0, 1.0, 10.0,
		0, TRUE, 0.0, 0.0, "C*", NULL
	) );
	g_signal_connect(
		self->scale_c, "value-changed",
		G_CALLBACK( lch_selector_on_value_changed ), self
	);
	self->scale_h = GTK_ADJUSTMENT( gimp_scale_entry_new(
		GTK_TABLE( table ), 0, 2, "h*", -1, -1,
		0.0, 0.0, 360.0, 1.0, 10.0,
		0, TRUE, 0.0, 0.0, "h*", NULL
	) );
	g_signal_connect(
		self->scale_h, "value-changed",
		G_CALLBACK( lch_selector_on_value_changed ), self
	);

	gtk_widget_show( table );
}


/* entry point */

G_MODULE_EXPORT const GimpModuleInfo* gimp_module_query( GTypeModule* module ) {
	static GimpModuleInfo info = {
		GIMP_MODULE_ABI_VERSION,
		"CIE LCh color selector",
		"y.fujii",
		"v0.1",
		"public domain",
		"2009"
	};
	return &info;
}

G_MODULE_EXPORT gboolean gimp_module_register( GTypeModule* module ) {
	lch_selector_register_type( module );
	return TRUE;
}
