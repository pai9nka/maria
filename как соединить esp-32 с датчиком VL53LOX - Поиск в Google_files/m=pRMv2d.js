"use strict";loaded_g_0(function(_){var window=this;
_.h("pRMv2d");
_.xMa(_.Mu,class extends _.yMa{static Ra(){return{}}constructor(){super();this.places=new Map;this.la=null;this.Ca=new Set;this.ya=new Set;this.xa=new Set;this.wa=new Set;this.da=!1;this.Oa=_.bSa(this.Ua,100,this)}pa(a){return this.places.get(a)}j(){return[...this.places.values()]}Ea(){this.da=!1;this.places.clear()}aa(...a){let b=!1;for(const c of a){const d=this.places.get(c.mid);d?(!d.latLng&&c.latLng&&(d.latLng=c.latLng,b=!0),!d.fprint&&c.fprint&&(d.fprint=c.fprint,b=!0),!d.categoryName&&c.categoryName&&
(d.categoryName=c.categoryName,b=!0),a=Object.fromEntries(Object.entries(c).filter(([e,f])=>!d[e]&&(f!==void 0||f!==""))),Object.assign(d,a)):this.places.set(c.mid,Object.assign({},c));b=!0}b&&this.Oa()}ba(a){this.la=a?Object.assign({},a):null;for(const b of this.ya)b(this.la)}Fa(a){this.ya.add(a);this.la&&a(this.la)}Ma(a){this.Ca.add(a);const b=this.j();b.length&&a(b)}Ia(a){this.xa.add(a);this.da&&this.j().filter(b=>!!b.latLng).length===0&&a()}Ya(a){this.wa.add(a);this.da&&a()}na(){this.da=!0;this.j().filter(a=>
!!a.latLng).length===0&&this.xa.forEach(a=>void a());this.wa.forEach(a=>void a())}Ua(){const a=this.j();if(a.length)for(const b of this.Ca)b(a)}});
_.q();
});
// Google Inc.
