#pragma once
/*
	L*C*h* color selector for GIMP
	by y.fujii <y-fujii at mimosa-pudica.net>, public domain
*/

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdint.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <cairo/cairo.h>
#include <libgimpcolor/gimpcolor.h>
#include <libgimpwidgets/gimpwidgets.h>
#include "color-space.hpp"


template<class T>
void unref( T*& obj ) {
	if( obj != 0 ) {
		g_object_unref( obj );
		obj = 0;
	}
}


struct LchSelector: GimpColorSelector {
	GtkAdjustment* scaleL;
	GtkAdjustment* scaleC;
	GtkAdjustment* scaleH;
	GtkDrawingArea* area;
	GdkPixbuf* pixbuf;
	double prevL;

	static void ctor( LchSelector* self ) {
		self->pixbuf = createColorMap( 1, 1, 0.0 );
		self->prevL = std::numeric_limits<double>::quiet_NaN();

		GtkWidget* vbox = gtk_vbox_new( FALSE, 0 );
		gtk_box_pack_start( GTK_BOX( self ), vbox, TRUE, TRUE, 0 );

		GtkWidget* table = gtk_table_new( 3, 3, FALSE );
		gtk_box_pack_start( GTK_BOX( vbox ), table, FALSE, FALSE, 0 );

		self->scaleL = GTK_ADJUSTMENT( gimp_scale_entry_new(
			GTK_TABLE( table ), 0, 0, "L*", -1, -1,
			0.0, 0.0, 100.0, 1.0, 10.0,
			0, TRUE, 0.0, 0.0, "L*", NULL
		) );
		g_signal_connect(
			self->scaleL, "value-changed",
			G_CALLBACK( onValueChanged ), self
		);
		self->scaleC = GTK_ADJUSTMENT( gimp_scale_entry_new(
			GTK_TABLE( table ), 0, 1, "C*", -1, -1,
			0.0, 0.0, 100.0, 1.0, 10.0,
			0, TRUE, 0.0, 0.0, "C*", NULL
		) );
		g_signal_connect(
			self->scaleC, "value-changed",
			G_CALLBACK( onValueChanged ), self
		);
		self->scaleH = GTK_ADJUSTMENT( gimp_scale_entry_new(
			GTK_TABLE( table ), 0, 2, "h*", -1, -1,
			0.0, 0.0, 360.0, 1.0, 10.0,
			0, TRUE, 0.0, 0.0, "h*", NULL
		) );
		g_signal_connect(
			self->scaleH, "value-changed",
			G_CALLBACK( onValueChanged ), self
		);

		self->area = GTK_DRAWING_AREA( gtk_drawing_area_new() );
		gtk_box_pack_start( GTK_BOX( vbox ), GTK_WIDGET( self->area ), TRUE, TRUE, 0 );
		g_signal_connect(
			self->area, "expose-event",
			G_CALLBACK( onExpose ), self
		);

		gtk_widget_show_all( vbox );
	}

	static void dtor( LchSelector* self ) {
		unref( self->pixbuf );
	}

	static void setColor( LchSelector* self, const GimpRGB* rgb, const GimpHSV* ) {
		color::Lab lab;
		color::Lch lch;
		color::convert( lab, *rgb );
		color::convert( lch, lab );

		setLch( self, lch );
	}

	static void onValueChanged( GtkAdjustment*, LchSelector* self ) {
		color::Lch lch;
		color::Lab lab;
		getLch( self, lch );
		color::convert( lab, lch );
		color::convert( self->rgb, lab );
		gimp_rgb_to_hsv( &self->rgb, &self->hsv );

		gimp_color_selector_color_changed( self );

		gtk_widget_queue_draw_area(
			GTK_WIDGET( self->area ), 0, 0,
			GTK_WIDGET( self->area )->allocation.width,
			GTK_WIDGET( self->area )->allocation.height
		);
	}

	static void onExpose( GtkDrawingArea* area, GdkEventExpose* ev, LchSelector* self ) {
		using namespace std;

		if( ev->count > 0 ) {
			return;
		}

		int size = min(
			GTK_WIDGET( area )->allocation.width,
			GTK_WIDGET( area )->allocation.height
		);
		color::Lch lch;
		getLch( self, lch );

		if( size != gdk_pixbuf_get_width( self->pixbuf ) || lch.l != self->prevL ) {
			unref( self->pixbuf );
			self->pixbuf = createColorMap( size, size, lch.l );
			self->prevL = lch.l;
		}
		gdk_draw_pixbuf(
			GTK_WIDGET( area )->window, NULL,
			self->pixbuf,
			0, 0, 0, 0,
			-1, -1,
			GDK_RGB_DITHER_NORMAL, 0, 0
		);

		unsigned x0, y0, x1, y1;
		toScreenPos( x0, y0, -lch.c, +lch.c, size );
		toScreenPos( x1, y1, +lch.c, -lch.c, size );
		gdk_draw_arc(
			GTK_WIDGET( area )->window, GTK_WIDGET( area )->style->white_gc,
			FALSE, x0, y0, x1 - x0, y1 - y0, 0, 360 * 64
		);

		toScreenPos( x0, y0, 0.0, 0.0, size );
		toScreenPos( x1, y1, 2.0 * cos( lch.h ), 2.0 * sin( lch.h ), size );
		gdk_draw_line(
			GTK_WIDGET( area )->window, GTK_WIDGET( area )->style->white_gc,
			x0, y0, x1, y1
		);

		/*
		cairo_t* ctx = gdk_cairo_create( GTK_WIDGET( area )->window );
		*/
	}

	static void setLch( LchSelector* self, const color::Lch& lch ) {
		gtk_adjustment_set_value( self->scaleL, lch.l * 100.0 );
		gtk_adjustment_set_value( self->scaleC, lch.c * 100.0 );
		gtk_adjustment_set_value( self->scaleH, lch.h * (180.0 / M_PI) );

		gtk_widget_queue_draw_area(
			GTK_WIDGET( self->area ), 0, 0,
			GTK_WIDGET( self->area )->allocation.width,
			GTK_WIDGET( self->area )->allocation.height
		);
	}
	
	static void getLch( LchSelector* self, color::Lch& lch ) {
		lch.l = self->scaleL->value / 100.0;
		lch.c = self->scaleC->value / 100.0;
		lch.h = self->scaleH->value * (M_PI / 180.0);
	}

	static GdkPixbuf* createColorMap( unsigned w, unsigned h, double l ) {
		GdkPixbuf* pixbuf = gdk_pixbuf_new( GDK_COLORSPACE_RGB, FALSE, 8, w, h );
		uint8_t* arr = gdk_pixbuf_get_pixels( pixbuf );
		unsigned stride = gdk_pixbuf_get_rowstride( pixbuf );

		for( unsigned y = 0; y < h; ++y ) {
			for( unsigned x = 0; x < w; ++x ) {
				color::Lab lab;
				lab.l = l;
				lab.a = x * (+2.0 / (w - 1)) - 1.0;
				lab.b = y * (-2.0 / (h - 1)) + 1.0;

				GimpRGB srgb;
				if( convert( srgb, lab ) ) {
					arr[x * 3 + y * stride + 0] = uint8_t( srgb.r * 255.0 ); 
					arr[x * 3 + y * stride + 1] = uint8_t( srgb.g * 255.0 );
					arr[x * 3 + y * stride + 2] = uint8_t( srgb.b * 255.0 );
				}
				else {
					arr[x * 3 + y * stride + 0] = 186;
					arr[x * 3 + y * stride + 1] = 186;
					arr[x * 3 + y * stride + 2] = 186;
				}
			}
		}
		return pixbuf;
	}

	static void toScreenPos( unsigned& sx, unsigned& sy, double rx, double ry, unsigned s ) {
		sx = unsigned( (+rx + 1.0) * ((s - 1) / 2.0) );
		sy = unsigned( (-ry + 1.0) * ((s - 1) / 2.0) );
	}
};


struct LchSelectorClass: GimpColorSelectorClass {
	typedef void (*disposeFunc)( GObject* );
	typedef void (*setColorFunc)( GimpColorSelector*, const GimpRGB*, const GimpHSV* );

	disposeFunc disposeSuper;

	static void ctor( LchSelectorClass* self ) {
		self->disposeSuper = G_OBJECT_CLASS( self )->dispose;
		G_OBJECT_CLASS( self )->dispose = disposeInstance;

		self->name = "Lch";
		self->help_id = "gimp-colorselector-lch";
		self->stock_id = GTK_STOCK_COLOR_PICKER;
		self->set_color = reinterpret_cast<setColorFunc>( &LchSelector::setColor );
	}

	static void disposeInstance( GObject* obj ) {
		LchSelector::dtor( reinterpret_cast<LchSelector*>( obj ) );
		reinterpret_cast<LchSelectorClass*>( G_OBJECT_GET_CLASS( obj ) )->disposeSuper( obj );
	}

	static GType register_( GTypeModule* module ) {
		const GTypeInfo info = { 
			sizeof( LchSelectorClass ),
			NULL,
			NULL,
			reinterpret_cast<GClassInitFunc>( &ctor ),
			NULL,
			NULL,
			sizeof( LchSelector ),
			0,  
			reinterpret_cast<GInstanceInitFunc>( &LchSelector::ctor ),
			NULL,
		};  
		return g_type_module_register_type(
			module,
			GIMP_TYPE_COLOR_SELECTOR,
			"LchSelector",
			&info,
			GTypeFlags( 0 )
		);
	}
};
