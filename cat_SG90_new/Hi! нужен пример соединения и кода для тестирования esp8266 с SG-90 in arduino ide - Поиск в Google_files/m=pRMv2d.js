"use strict";loaded_g_0(function(_){var window=this;
_.h("pRMv2d");
_.ELa(_.Du,class extends _.FLa{static Qa(){return{}}constructor(){super();this.places=new Map;this.ea=null;this.za=new Set;this.wa=new Set;this.va=new Set;this.oa=new Set;this.da=!1;this.La=_.bSa(this.Na,100,this)}la(a){return this.places.get(a)}j(){return[...this.places.values()]}Da(){this.da=!1;this.places.clear()}aa(...a){let b=!1;for(const c of a){const d=this.places.get(c.mid);d?(!d.latLng&&c.latLng&&(d.latLng=c.latLng,b=!0),!d.fprint&&c.fprint&&(d.fprint=c.fprint,b=!0),!d.categoryName&&c.categoryName&&
(d.categoryName=c.categoryName,b=!0),a=Object.fromEntries(Object.entries(c).filter(([e,f])=>!d[e]&&(f!==void 0||f!==""))),Object.assign(d,a)):this.places.set(c.mid,Object.assign({},c));b=!0}b&&this.La()}ba(a){this.ea=a?Object.assign({},a):null;for(const b of this.wa)b(this.ea)}Ea(a){this.wa.add(a);this.ea&&a(this.ea)}Ia(a){this.za.add(a);const b=this.j();b.length&&a(b)}Fa(a){this.va.add(a);this.da&&this.j().filter(b=>!!b.latLng).length===0&&a()}Sa(a){this.oa.add(a);this.da&&a()}ka(){this.da=!0;this.j().filter(a=>
!!a.latLng).length===0&&this.va.forEach(a=>void a());this.oa.forEach(a=>void a())}Na(){const a=this.j();if(a.length)for(const b of this.za)b(a)}});
_.q();
});
// Google Inc.
