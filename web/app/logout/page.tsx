"use client"
const handleLogout = () => {
  window.location.href = '/api/auth/logout';
};

export default function logout() {
  return <button onClick={handleLogout}>Log Out</button>
}
